#pragma once
#include <open62541/types.h>
#include <string>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

static std::string extractValidOPCUANodes(const std::string& str) {
	std::string validChars;
	if (str.empty()) return validChars;
	// 处理后续字符：允许字母、数字、下划线、空格和竖线
	for (size_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if ((c >= '0' && c <= '9') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c == '_') ||
			(c == '-') ||
			(c == ' ') ||
			(c == '.') ||
			(c == '|')) {
			validChars.push_back(c);
		}
		else {
			// 遇到其他无效字符则停止处理
			return validChars;
		}
	}

	return validChars;
}

static std::string trimNonPrintableAscii(const std::string& input) {
	std::string result;
	for (char c : input) {
		unsigned char uc = static_cast<unsigned char>(c);
		// 只保留ASCII可打印字符（32-126），遇到非可打印字符则终止
		if (uc < 32 || uc > 126) {
			break;
		}
		result += c;
	}
	return result;
}

static bool stringToUint16(const QString& str, uint16_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	unsigned int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_UINT16_MAX && value >= UA_UINT16_MIN) {
		result = static_cast<uint16_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToUint32(const QString& str, uint32_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	unsigned int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_UINT32_MAX && value >= UA_UINT32_MIN) {
		result = static_cast<uint32_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToUint64(const QString& str, uint64_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	unsigned int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_UINT64_MAX && value >= UA_UINT64_MIN) {
		result = static_cast<uint64_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToInt16(const QString& str, int16_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_INT16_MAX && value >= UA_INT16_MIN) {
		result = static_cast<int16_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToInt32(const QString& str, int32_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_INT32_MAX && value >= UA_INT32_MIN) {
		result = static_cast<uint32_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToInt64(const QString& str, int64_t& result)
{
	bool ok;
	// 转换为无符号整数，基数默认为10（可指定为16表示十六进制）
	int value = str.toUInt(&ok, 10);

	// 检查转换是否成功且值在 uint16_t 范围内
	if (ok && value <= UA_INT64_MAX && value >= UA_INT64_MIN) {
		result = static_cast<int64_t>(value);
		return true;
	}
	return false;  // 转换失败或超出范围
}

static bool stringToFloat(const QString& str, float& result) {
	bool ok = false;
	result = str.toFloat(&ok); // ok为true表示转换成功，false失败

	// 额外处理：空字符串、纯空格字符串也判定为非法
	if (str.trimmed().isEmpty()) {
		ok = false;
	}

	return ok;
}

static bool stringToDouble(const QString& str, double& result) {
	QString trimmedStr = str.trimmed();
	if (trimmedStr.isEmpty()) return false;

	// 正则表达式：匹配完整的double格式（支持正负、整数、小数、科学计数法）
	QRegularExpression reg(R"(^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$)");
	QRegularExpressionMatch match = reg.match(trimmedStr);
	if (!match.hasMatch()) {
		return false; // 格式非法（含多余字符）
	}

	// 格式合法后再转换，确保数值在double范围
	bool ok = false;
	result = trimmedStr.toDouble(&ok);
	// 排除inf/nan
	if (ok && (qIsInf(result) || qIsNaN(result))) {
		ok = false;
	}
	return ok;
}