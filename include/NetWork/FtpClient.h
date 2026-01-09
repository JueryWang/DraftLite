#pragma once
//curl - T C : \Users\WG\Desktop\Labubu.nc ftp ://192.168.6.6:21/Share_files_anonymity/Labubu.nc --user anonymous: --connect-timeout 0.1 --max-time 1
#include <vector>
#include <string>
#include <QString>
#include <QProcess>
#include <curl/curl.h>

static size_t file_read_callback(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t list_write_callback(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr);
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
int ftp_get_list(CURL* curl, const char* ftp_url, struct FtpFile* list);
std::vector<std::string> ftp_get_dir_list(CURL* curl, const char* base_url);
std::string extractFTPFileName(const QString& fileName);

class FtpClient
{
public:
	static void SetFtpUrl(const QString& url);
	static void UploadFile(const QString& content, const QString& ftpFileDir);
	static QString DownLoadFile(const QString& ftpFile);
	static void CleanRemoteDirectory(const std::string& fileDir);
private:
	static QString ftpUrl;
	static QProcess curlCommander;
};
