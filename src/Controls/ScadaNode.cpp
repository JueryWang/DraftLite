#include "Controls/ScadaNode.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/GlobalPLCVars.h"
#include <iostream>

ScadaNode::ScadaNode()
{
	lastExecTime = std::chrono::system_clock::now();
}

ScadaNode::~ScadaNode()
{
	ScadaScheduler::GetInstance()->EraseNode(this);
}

void ScadaNode::SetUpdateRate(int rateInMs)
{
	PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[bindTag]);
	if (plcInfo)
	{
		plcInfo->lastUpdateTimeStamp = std::chrono::system_clock::now();
		plcInfo->collectionInterval.store(rateInMs, std::memory_order_release);
	}
}


void ScadaNode::BindParam(const std::string& tag)
{
	PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[tag]);
	bindTag = tag;
	if (plcInfo)
	{
		switch (plcInfo->dataType)
		{
			case AtomicVarType::BOOL:
			{
				AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::WORD:
			{
				AtomicVar<PLC_TYPE_WORD>* var = static_cast<AtomicVar<PLC_TYPE_WORD>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::DWORD:
			{
				AtomicVar<PLC_TYPE_DWORD>* var = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::LWORD:
			{
				AtomicVar<PLC_TYPE_LWORD>* var = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::INT:
			{
				AtomicVar<PLC_TYPE_INT>* var = static_cast<AtomicVar<PLC_TYPE_INT>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::DINT:
			{
				AtomicVar<PLC_TYPE_DINT>* var = static_cast<AtomicVar<PLC_TYPE_DINT>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::LINT:
			{
				AtomicVar<PLC_TYPE_LINT>* var = static_cast<AtomicVar<PLC_TYPE_LINT>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::REAL:
			{
				AtomicVar<PLC_TYPE_REAL>* var = static_cast<AtomicVar<PLC_TYPE_REAL>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::LREAL:
			{
				AtomicVar<PLC_TYPE_LREAL>* var = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::STRING:
			{
				AtomicVar<PLC_TYPE_STRING>* var = static_cast<AtomicVar<PLC_TYPE_STRING>*>(plcInfo->bindVar);
				this->BindParam(var);
				break;
			}
			case AtomicVarType::ARRAY_BOOL:
			{
				if (tag == "gvlGlobalData.stIOGearChamferMachine.axPCFileFTP")
				{
					param = &g_stationPCFileFTP;
					varType = AtomicVarType::ARRAY_BOOL;
				}
				break;
			}
		default:
			break;
		}
	}
	else
	{
		param = nullptr;
		varType = AtomicVarType::None;
	}
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_BOOL>* address)
{
	param = address;
	varType = AtomicVarType::BOOL;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_WORD>* address)
{
	param = address;
	varType = AtomicVarType::WORD;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_DWORD>* address)
{
	param = address;
	varType = AtomicVarType::DWORD;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_LWORD>* address)
{
	param = address;
	varType = AtomicVarType::LWORD;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_INT>* address)
{
	param = address;
	varType = AtomicVarType::INT;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_DINT>* address)
{
	param = address;
	varType = AtomicVarType::DINT;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_LINT>* address)
{
	param = address;
	varType = AtomicVarType::LINT;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_REAL>* address)
{
	param = address;
	varType = AtomicVarType::REAL;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_LREAL>* address)
{
	param = address;
	varType = AtomicVarType::LREAL;
}

void ScadaNode::BindParam(AtomicVar<PLC_TYPE_STRING>* address)
{
	param = address;
	varType = AtomicVarType::STRING;
}