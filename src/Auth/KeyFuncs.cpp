#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QString>
#include "Auth/KeyFuncs.h"
#include "Auth/WG_Authorization.h"

bool ExecuteCmdWithoutConsole(const std::wstring& cmd, std::string& output, std::string& error)
{
	output.clear();
	error.clear();

	// 1. 创建管道（用于捕获 cmd 输出）
	HANDLE hPipeRead = nullptr;  // 管道读端（主线程读取）
	HANDLE hPipeWrite = nullptr; // 管道写端（cmd 进程输出重定向到这里）
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;    // 子进程可继承此管道句柄
	sa.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
		error = "创建管道失败，错误码：" + std::to_string(GetLastError());
		return false;
	}

	// 2. 设置 cmd 进程启动信息（隐藏控制台 + 重定向输出）
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(STARTUPINFOW);
	si.dwFlags = STARTF_USESTDHANDLES;  // 使用自定义标准输入/输出句柄
	si.hStdOutput = hPipeWrite;         // 标准输出重定向到管道写端
	si.hStdError = hPipeWrite;          // 标准错误也重定向到管道（可选，按需关闭）
	si.hStdInput = INVALID_HANDLE_VALUE; // 无需输入

	PROCESS_INFORMATION pi = { 0 };       // 进程信息（用于等待进程结束）

	// 命令格式：cmd.exe /c "目标命令"（支持带空格、管道符的复杂命令）
	std::wstring cmdLine = L"cmd.exe /c \"" + cmd + L"\"";

	// 3. 创建进程（关键：CREATE_NO_WINDOW 隐藏控制台）
	if (!CreateProcessW(
		nullptr,                          // 应用程序路径（nullptr 表示使用 cmdLine 第一个参数）
		const_cast<LPWSTR>(cmdLine.c_str()), // 命令行（需可修改，故用 const_cast）
		nullptr,                          // 进程安全描述符（默认）
		nullptr,                          // 线程安全描述符（默认）
		TRUE,                             // 允许子进程继承管道句柄
		CREATE_NO_WINDOW,                 // 隐藏控制台窗口（核心标志）
		nullptr,                          // 环境变量（默认）
		nullptr,                          // 工作目录（默认）
		&si,                              // 启动信息
		&pi                               // 输出进程信息
	)) {
		error = "创建进程失败，错误码：" + std::to_string(GetLastError());
		SAFE_CLOSE_HANDLE(hPipeRead);
		SAFE_CLOSE_HANDLE(hPipeWrite);
		return false;
	}

	// 4. 关闭管道写端（子进程已继承，主线程无需保留）
	SAFE_CLOSE_HANDLE(hPipeWrite);

	// 5. 从管道读端读取命令输出（循环读取，直到子进程结束）
	const DWORD BUFFER_SIZE = 4096;
	std::vector<BYTE> buffer(BUFFER_SIZE);
	DWORD bytesRead = 0;

	while (ReadFile(hPipeRead, buffer.data(), BUFFER_SIZE, &bytesRead, nullptr) && bytesRead > 0) {
		// Windows cmd 输出默认编码为 GBK（中文系统）/ CP437（英文系统），转为 UTF-8
		int utf8Len = MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPCSTR>(buffer.data()), bytesRead, nullptr, 0);
		std::wstring wideStr(utf8Len, 0);
		MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPCSTR>(buffer.data()), bytesRead, &wideStr[0], utf8Len);

		int ansiLen = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), utf8Len, nullptr, 0, nullptr, nullptr);
		std::string utf8Str(ansiLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), utf8Len, &utf8Str[0], ansiLen, nullptr, nullptr);

		output += utf8Str;
	}

	// 6. 等待进程结束（避免僵尸进程）
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 7. 释放资源（逆序释放）
	SAFE_CLOSE_HANDLE(hPipeRead);
	SAFE_CLOSE_HANDLE(pi.hThread);
	SAFE_CLOSE_HANDLE(pi.hProcess);

	// 检查读取管道是否出错
	if (GetLastError() != ERROR_BROKEN_PIPE) {
		error = "读取管道失败，错误码：" + std::to_string(GetLastError());
		return false;
	}

	return true;
}

std::vector<std::string> SplitByCRLF(const std::string& str)
{
	std::vector<std::string> lines;
	size_t start = 0;
	size_t pos = 0;

	// 遍历字符串，查找所有 \r\n 位置
	while ((pos = str.find("\r\n", start)) != std::string::npos) {
		// 提取当前行（从 start 到 pos，不含 \r\n）
		lines.push_back(str.substr(start, pos - start));
		start = pos + 2; // 跳过 \r\n（2 个字符），更新下一行起始位置
	}

	// 提取最后一行（若字符串末尾无 \r\n）
	if (start < str.size()) {
		lines.push_back(str.substr(start));
	}

	for (std::string& line : lines)
	{
		auto it = std::remove_if(line.begin(), line.end(), [](char c) {
			// 筛选条件：\r、普通空格、制表符 均移除
			return c == '\r' || c == ' ' || c == '\t';
			});

		// erase：删除尾迭代器之后的无效字符
		line.erase(it, line.end());
	}

	return lines;
}

QString GetKeyMatchDifference(char* currentChipID, char* currentUUID)
{
	char* originChipId = GetAuthChipId();
	char* originHostId = GetAuthPCUUID();
	char* originVeriCode = GetAuthVeriCode();
	QString outMessage = QString("原始验证信息:\r\n加密狗ID:%1\r\nPC信息:%2\r\n校验码信息:%3\r\n当前验证信息:\r\n加密狗ID:%4\r\nPC信息:%5\r\n校验码信息:").arg(originChipId).arg(originHostId).arg(originVeriCode).arg(currentChipID).arg(currentUUID);

	QByteArray temp = outMessage.toLocal8Bit();
	char* result = temp.data();
	return outMessage;
}
