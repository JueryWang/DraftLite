#define _CRT_SECURE_NO_WARNINGS
#include "NetWork/FtpClient.h"
#include "Common/ProgressInfo.h"
#include <sys/stat.h>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <QString>
QString FtpClient::ftpUrl;
QProcess FtpClient::curlCommander;

CURL* curl = nullptr;

struct FtpFile {
	char* buffer;	//存储列表内容
	size_t size;	//内容长度
};

std::string extractFTPFileName(const QString& _file)
{
	QStringList parts = _file.split("/");
	QString fileName = parts.last();
	QString trimmedFileName = fileName.replace(" ", "");
	QFileInfo fileInfo(trimmedFileName);
	QString suffix = fileInfo.suffix(); // 结果："txt"
	fileName = fileInfo.baseName() + ".cnc";
	fileName = "Share_files_anonymity/" + fileName.trimmed();

	return fileName.toStdString();
}


void FtpClient::SetFtpUrl(const QString& url)
{
	ftpUrl = url;
	if (curl == nullptr)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 300L);
	}
}

void FtpClient::UploadFile(const QString& content, const QString& ftpFileDir)
{
	QDir currentDir = QDir::current();
	if (!currentDir.exists("temp"))
	{
		currentDir.mkdir("temp");
	}
	QFile file("./temp/uplaod.temp");
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		file.write(content.toLocal8Bit());
		file.close();
	}

	const char* local_file_path = "./temp/uplaod.temp";
	QString fullFtpUrl = ftpUrl + "/" + ftpFileDir;
	QByteArray s = fullFtpUrl.toLocal8Bit();
	const char* ftp_upload_url = s.data();

	FILE* local_file = fopen(local_file_path, "rb");
	if (!local_file) {
		fprintf(stderr, "cannot open file：%s\n", local_file_path);
	}

	// 获取本地文件大小（用于 CURLOPT_INFILESIZE）
	struct stat file_info;
	if (stat(local_file_path, &file_info) != 0) {
		fclose(local_file);
	}

	if (curl)
	{
		curl_easy_reset(curl);
		curl_easy_setopt(curl, CURLOPT_URL, ftp_upload_url);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, file_read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, local_file);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE, (curl_off_t)file_info.st_size);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "anonymous:");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 100L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);
		curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "upload failed: " << curl_easy_strerror(res) << std::endl;
			g_file_logger->critical("上传CNC文件失败:,error code:{}  ||调用函数{}", curl_easy_strerror(res), __FUNCTION__);
		}
	}
	fclose(local_file);
}

QString FtpClient::DownLoadFile(const QString& ftpFile)
{
	curlCommander.setProgram("curl");
	QStringList arguments;
	QString fullFtpUrl = ftpUrl + "/" + ftpFile;
	arguments << fullFtpUrl;
	arguments << "--user";
	arguments << "anonymous:";
	qDebug() << "执行命令:" << arguments.join(" ");

	curlCommander.setArguments(arguments);
	curlCommander.start();
	curlCommander.waitForFinished();
	QString output = QString::fromLocal8Bit(curlCommander.readAllStandardOutput());
	return output;
}

void FtpClient::CleanRemoteDirectory(const std::string& fileDir)
{
	struct curl_slist* commands = NULL;
	if (curl)
	{
		curl_easy_reset(curl);

		CURLcode res;
		auto list = ftp_get_dir_list(curl, QString(FtpClient::ftpUrl + "/" + QString::fromStdString(fileDir)).toLocal8Bit());

		char chcmd[256];
		snprintf(chcmd, sizeof(chcmd), "CWD /%s", fileDir.c_str());
		commands = curl_slist_append(commands, chcmd);
		for (const std::string& file : list)
		{
			char delCmd[512];
			snprintf(delCmd, sizeof(delCmd), "DELE %s", file.c_str());
			commands = curl_slist_append(commands, delCmd);
		}

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

		QByteArray byteArray = FtpClient::ftpUrl.toLocal8Bit();
		char* ftpstr = byteArray.data();
		// Set the base URL
		curl_easy_setopt(curl, CURLOPT_URL, ftpstr);

		// Set the commands to execute
		curl_easy_setopt(curl, CURLOPT_QUOTE, commands);

		// Use active mode if passive mode fails
		curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
		curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 5L);


		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			// 可选：获取服务器返回的具体错误码
			long responseCode = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
			g_file_logger->warn("清空FTP文件夹失败:{},error code:{}  ||调用函数{}", curl_easy_strerror(res), responseCode,__FUNCTION__);
		}
		// Cleanup
		curl_slist_free_all(commands);
	}
}

size_t file_read_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
	FILE* file = (FILE*)stream;
	if (!file) return 0;

	return fread(ptr, size, nmemb, file);
}

size_t list_write_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
	struct FtpFile* out = (struct FtpFile*)stream;
	const size_t total = size * nmemb;

	// 重新分配内存并拼接数据
	char* new_buf = (char*)realloc(out->buffer, out->size + total + 1);
	if (!new_buf) {
		fprintf(stderr, "内存分配失败\n");
		return 0; // 分配失败，停止传输
	}

	out->buffer = new_buf;
	memcpy(out->buffer + out->size, ptr, total);
	out->size += total;
	out->buffer[out->size] = '\0'; // 确保字符串结束

	return total;
}

size_t debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
{
	(void)handle; // 防止未使用警告
	(void)userptr;

	return 0;
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	(void)contents;
	(void)userp;
	return size * nmemb;
}

int ftp_get_list(CURL* curl, const char* ftp_url, struct FtpFile* list)
{
	if (curl != nullptr)
	{
		CURLcode res;
		curl_easy_reset(curl);

		// 初始化列表结构体
		list->buffer = NULL;
		list->size = 0;

		// 设置 FTP URL（使用 LIST 命令获取详细列表）
		curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
		// 设置回调函数处理列表数据
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, list_write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, list);
		// 启用被动模式（多数 FTP 服务器需要）
		curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
		curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0L);
		// 设置连接超时为 10 秒
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000L);
		// 设置整个操作超时为 30 秒
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 3000L);

		// 执行请求
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			g_file_logger->warn("清空FTP文件夹失败:{},error code:{}  ||调用函数{}", curl_easy_strerror(res), __FUNCTION__);
			return 0;
		}
	}
	return 1;
}

std::vector<std::string> ftp_get_dir_list(CURL* curl, const char* base_url)
{
	struct FtpFile list = { 0 };
	char* token;
	char* saveptr;
	std::vector<std::string> fileList;

	// 1. 确保路径以 / 结尾
	char list_url[512];
	snprintf(list_url, sizeof(list_url), "%s/", base_url);

	if (!ftp_get_list(curl, list_url, &list)) {
		goto cleanup;
	}

	if (list.buffer != NULL)
	{
		token = strtok_s(list.buffer, "\r\n", &saveptr);

		while (token != NULL) {
			if (token[0] == '\0') {
				token = strtok_s(NULL, "\r\n", &saveptr);
				continue;
			}

			// --- 核心修复逻辑：保留空格 ---
			// 标准 LIST 格式通常有 9 列，文件名是第 9 列及其之后的所有内容
			int column = 0;
			char* p = token;
			// 跳过前 8 个字段
			while (*p && column < 8) {
				while (*p && *p == ' ') p++; // 跳过空格
				if (*p && *p != ' ') {
					column++;
					while (*p && *p != ' ') p++; // 跳过非空格内容
				}
			}
			// 此时 p 指向第 8 个字段后的空格，跳过这些空格即为完整文件名
			while (*p && *p == ' ') p++;

			std::string filename(p);

			// 过滤掉当前目录和父目录
			if (filename == "." || filename == "..") {
				token = strtok_s(NULL, "\r\n", &saveptr);
				continue;
			}

			if (!filename.empty()) {
				fileList.push_back(filename);
			}

			token = strtok_s(NULL, "\r\n", &saveptr);
		}
	}

cleanup:
	if (list.buffer) free(list.buffer);
	return fileList;
}