#pragma once
#include "Common/AtomicVariables.h"
#include "Controls/GlobalPLCVars.h"
#include <vector>
#include <string>
#include <variant>

class CraftParamConfig
{
public:
	CraftParamConfig();
	~CraftParamConfig();
	CraftParamConfig(const CraftParamConfig& other)
	{
		this->alias = other.alias;
		this->remark = other.remark;
		this->preSetVal = other.preSetVal;
		this->plcInfo = other.plcInfo;
	}

	CraftParamConfig& operator=(const CraftParamConfig other)
	{
		if (this == &other) {
			return *this;
		}

		this->alias = other.alias;
		this->plcInfo = other.plcInfo;
		this->preSetVal = other.preSetVal;
		this->remark = other.remark;
	}

public:
	std::string alias;
	PLCParam_ProtocalOpc* plcInfo = nullptr;
	std::variant<bool, uint16_t, uint32_t, uint64_t, int16_t, int32_t, int64_t, float, double, std::string> preSetVal;
	std::string remark;
};

typedef std::vector<CraftParamConfig> CraftConfigItems;