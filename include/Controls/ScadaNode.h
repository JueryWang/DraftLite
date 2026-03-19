#pragma once
#include "Common/AtomicVariables.h"
#include <functional>

#include <string>
#include <chrono>

enum class ScadaNodeType
{
	SLabel,
	SPubtton,
	SSliderBar,
};

class ScadaNode
{
	friend class ScadaScheduler;
public:
	ScadaNode();
	~ScadaNode();

	void BindParam(const std::string& tag);

	void SetUpdateRate(int rateInMs);
	void SetBindTag(const std::string& tag) { bindTag = tag;}

	inline PLC_TYPE_BOOL GetBool()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(this->param);
			return var->GetValue();
		}
		return false;
	}
	inline PLC_TYPE_WORD GetWord()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_WORD>* var = static_cast<AtomicVar<PLC_TYPE_WORD>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_DWORD GetDword()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_DWORD>* var = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_LWORD GetLword()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_LWORD>* var = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_INT GetInt()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_INT>* var = static_cast<AtomicVar<PLC_TYPE_INT>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_DINT GetDint()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_DINT>* var = static_cast<AtomicVar<PLC_TYPE_DINT>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_LINT GetLint()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_LINT>* var = static_cast<AtomicVar<PLC_TYPE_LINT>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_REAL GetReal()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_REAL>* var = static_cast<AtomicVar<PLC_TYPE_REAL>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}
	inline PLC_TYPE_LREAL GetLreal()
	{
		if (this->param)
		{
			AtomicVar<PLC_TYPE_LREAL>* var = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(this->param);
			return var->GetValue();
		}
		return 0;
	}

protected:
	virtual void UpdateNode() { if (updateCallback) { updateCallback(); } };
	void BindParam(AtomicVar<PLC_TYPE_BOOL>* address);
	void BindParam(AtomicVar<PLC_TYPE_WORD>* address);
	void BindParam(AtomicVar<PLC_TYPE_DWORD>* address);
	void BindParam(AtomicVar<PLC_TYPE_LWORD>* address);
	void BindParam(AtomicVar<PLC_TYPE_INT>* address);
	void BindParam(AtomicVar<PLC_TYPE_DINT>* address);
	void BindParam(AtomicVar<PLC_TYPE_LINT>* address);
	void BindParam(AtomicVar<PLC_TYPE_REAL>* addrees);
	void BindParam(AtomicVar<PLC_TYPE_LREAL>* address);
	void BindParam(AtomicVar<PLC_TYPE_STRING>* address);

public:
	std::chrono::system_clock::time_point lastExecTime;
	std::string bindTag;
	std::function<void(void*)> valueChangedCallback = nullptr;
	std::function<void()> updateCallback = nullptr;
	AtomicVarType varType = AtomicVarType::None;
	ScadaNodeType nodeType;
	union
	{
		PLC_TYPE_BOOL lastBool;
		PLC_TYPE_WORD lastWord;
		PLC_TYPE_DWORD lastDWord;
		PLC_TYPE_LWORD lastLWord;
		PLC_TYPE_INT lastInt;
		PLC_TYPE_DINT lastDInt;
		PLC_TYPE_LINT lastLInt;
		PLC_TYPE_REAL lastReal;
		PLC_TYPE_LREAL lastLReal;
	}lastValue;

protected:
	void* param = nullptr;
};