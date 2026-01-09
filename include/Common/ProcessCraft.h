#pragma once
#include "../OGL/glm/glm.hpp"
#include <variant>

enum class GeomDirection
{
	CW,
	CCW,
	AtLine
};

enum class LeadType
{
	None,
	Linear,
	Arc,
};

struct LeadLineParams
{
	LeadType leadType;
	float length;
	int angle;
};

struct LeadLine
{
	glm::vec3 start;
	glm::vec3 end;

	LeadType tRule;
	float length;
	int angle;
	int radius;
};

//struct CraftPreConfigDefines
//{
//	std::string alias;
//	PLCParam_ProtocalOpc* plcInfo;
//	std::variant<bool, uint16_t, uint32_t, uint64_t, int16_t, int32_t, int64_t, float, double, std::string> value;
//	std::string remark;
//};