#pragma once
#include<atomic>
#include<string>
#include<iostream>
#include<cstdint>
#include<stdexcept>
#include <limits>

/// <summary>
/// PLC types Conver to C++
/// </summary>
typedef bool    PLC_TYPE_BOOL;
typedef uint16_t PLC_TYPE_WORD;
typedef uint32_t PLC_TYPE_DWORD;
typedef uint64_t PLC_TYPE_LWORD;
typedef int16_t PLC_TYPE_INT;
typedef int32_t PLC_TYPE_DINT;
typedef int64_t PLC_TYPE_LINT;
typedef float   PLC_TYPE_REAL;
typedef double PLC_TYPE_LREAL;
typedef char* PLC_TYPE_STRING;

enum class AtomicVarType
{
	None,
	BOOL,       //bool
	WORD,       //uint16_t
	DWORD,      //uint32_t
	LWORD,      //uint64_t
	INT,        //int16_t
	DINT,       //int32_t
	LINT,       //int64_t
	REAL,       //float
	LREAL,      //double
	STRING      //string
};

/// <summary>
/// 多线程使用原子语义的变量
/// </summary>
///
template <typename T>
struct AtomicVar
{
	// 构造函数：初始化原子变量
	AtomicVar(T initialValue) : val(initialValue) {}

	// 赋值操作：将值写入原子变量（使用 release 内存序）
	AtomicVar<T>& operator=(T newValue)
	{
		val.store(newValue, std::memory_order_release);
		return *this; // 支持链式赋值
	}

	// 隐式转换为 T 类型（使用 acquire 内存序读取）
	operator T() const
	{
		return val.load(std::memory_order_acquire);
	}

	bool operator==(const AtomicVar<T>& other) const
	{
		// 读取当前对象和另一个对象的值，用 T 的 == 比较
		return this->GetValue() == other.GetValue();
	}

	// 显式获取值的方法（与隐式转换语义一致）
	T GetValue() const
	{
		return val.load(std::memory_order_acquire);
	}

private:
	std::atomic<T> val; // 原子变量存储
};

template<>
struct AtomicVar<char*> {

	AtomicVar(const char* initialValue) {
		if (initialValue) {
			size_t len = strlen(initialValue) + 1;
			char* newVal = new char[len];
			strcpy(newVal, initialValue);
			val.store(newVal, std::memory_order_release);
		}
		else {
			val.store(nullptr, std::memory_order_release);
		}
	}

	// 拷贝构造函数 - 深拷贝
	AtomicVar(const AtomicVar<char*>& other) {
		const char* otherVal = other.val.load(std::memory_order_acquire);
		if (otherVal) {
			size_t len = strlen(otherVal) + 1;
			length = len - 1;
			char* newVal = new char[len];
			strcpy(newVal, otherVal);
			val.store(newVal, std::memory_order_release);
		}
		else {
			val.store(nullptr, std::memory_order_release);
		}
	}

	// 赋值操作 - 深拷贝
	AtomicVar<char*>& operator=(const char* newValue) {
		char* oldVal = val.load(std::memory_order_acquire);

		if (newValue) {
			size_t len = strlen(newValue) + 1;
			length = len - 1;
			char* newVal = new char[len];
			strcpy(newVal, newValue);
			val.store(newVal, std::memory_order_release);
		}
		else {
			val.store(nullptr, std::memory_order_release);
		}
		delete[] oldVal;
		return *this;
	}

	bool operator==(const AtomicVar<char*>& other) const {
		const char* thisVal = this->GetValue();
		const char* otherVal = other.GetValue();

		// 处理 nullptr 情况
		if (!thisVal && !otherVal) return true;  // 两者都为nullptr
		if (!thisVal || !otherVal) return false; // 其中一个为nullptr

		// 先比较长度（优化：长度不同直接不等）
		if (this->length != other.length) return false;

		// 长度相同再比较内容
		return strcmp(thisVal, otherVal) == 0;
	}

public:
	void SetValue(const char* newValue, int length)
	{
		char* oldVal = val.load(std::memory_order_acquire);

		if (newValue)
		{
			this->length = length;
			char* newVal = new char[length + 1];
			strncpy(newVal, newValue, length);
			newVal[length] = '\0';
			val.store(newVal, std::memory_order_release);
		}
		else {
			val.store(nullptr, std::memory_order_release);
		}
		delete[] oldVal;
	}

	// 析构函数 - 释放内存
	~AtomicVar() {
		char* oldVal = val.load(std::memory_order_acquire);
		delete[] oldVal;
	}

	// 隐式转换为 const char*
	operator const char* () const {
		return val.load(std::memory_order_acquire);
	}

	// 获取值
	const char* GetValue() const {
		return val.load(std::memory_order_acquire);
	}
private:
	std::atomic<char*> val;

public:
	int length;
};