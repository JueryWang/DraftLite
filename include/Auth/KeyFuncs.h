#pragma once

#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <QString>

#define SAFE_CLOSE_HANDLE(h) if (h != INVALID_HANDLE_VALUE && h != nullptr) { CloseHandle(h); h = nullptr; }
//获取本机UUID
bool ExecuteCmdWithoutConsole(const std::wstring& cmd, std::string& output, std::string& error);

std::vector<std::string> SplitByCRLF(const std::string& str);

QString GetKeyMatchDifference(char* currentChipID, char* currentUUID);