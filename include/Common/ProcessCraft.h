#pragma once
#include "../OGL/glm/glm.hpp"
#include <variant>

enum class GeomDirection
{
	CW,
	CCW,
	AtLine
};

enum class LineType
{
	SOLID,
	DASHD
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

enum class MillingDirection
{
	CW,		//顺铣
	CCW,	//逆铣
	Any     //任意方向
};

struct RoughingParamSettings
{
	double stepover = 5; //行距
	double allowance = 0.01;//余量
	double tolerance = 0.01;
	double toolRadius = 10;//刀具半径
	MillingDirection direction;
};

//struct CraftPreConfigDefines
//{
//	std::string alias;
//	PLCParam_ProtocalOpc* plcInfo;
//	std::variant<bool, uint16_t, uint32_t, uint64_t, int16_t, int32_t, int64_t, float, double, std::string> value;
//	std::string remark;
//};