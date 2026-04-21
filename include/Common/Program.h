#pragma once
#include "glm/glm.hpp"
#include "Graphics/AABB.h"
#include "Controls/GlobalPLCVars.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <Auth/dongle/D8.h>
#include <mutex>
#include <QMainWindow>
#include <QSettings>
#include <map>
#include <vector>

#define KEY_AUTH "8D0C9B361F74E5BE"
class MainLayer;

namespace CNCSYS
{
	class EntityVGPU;
	class CanvasGPU;
}
class Path2D;

struct SimulateStatus
{
	CNCSYS::AABB platformSize;
	CNCSYS::AABB objectRange;
	glm::vec3 toolPos;
	glm::vec3 wcsAnchor;
	int ncstep;
	double zoom;
	double Zup;
	double Zdown;
	PLC_TYPE_LREAL toolRadius = 0.0f;   //刀具半径补偿
	PLC_TYPE_LREAL toolDistance = 0.0f; //进刀距离
	double velocity = 100;
	double acceleration = 1000;
	double deceleartion = -1000;
	double jerk = 0.0f;
	double XAxisStart = 0.0;
	double YAxisStart = 0.0;
	double ZAxisStart = 0.0;
	double AAxisStart = 0.0;
	double BAxisStart = 0.0;
	double CAxisStart = 0.0;
	double UAxisStart = 0.0;
	double VAxisStart = 0.0;
	double WAxisStart = 0.0;
	double PAxisStart = 0.0;
	double QAxisStart = 0.0;
	double totalPath = 0.0;
	double idlePath = 0.0;
	bool openZ = false;  //开启Z轴
	// 默认构造函数
	SimulateStatus()
		: toolPos(0.0f),
		wcsAnchor(0.0f),
		zoom(1.0f),
		Zup(0.0f),
		Zdown(0.0f),
		ncstep(0)
	{

	}

	SimulateStatus(const CNCSYS::AABB& platform,
		const CNCSYS::AABB& canvas,
		const glm::vec3& tool,
		const glm::vec3& anchor,
		float z,
		float zUp,
		float zDown)
		: platformSize(platform),
		objectRange(canvas),
		toolPos(tool),
		wcsAnchor(anchor),
		zoom(z),
		Zup(zUp),
		Zdown(zDown),
		ncstep(0)
	{
	}

	SimulateStatus(const SimulateStatus& other)
		: platformSize(other.platformSize),
		objectRange(other.objectRange),
		toolPos(other.toolPos),
		wcsAnchor(other.wcsAnchor),
		zoom(other.zoom),
		Zup(other.Zup),
		Zdown(other.Zdown),
		ncstep(other.ncstep)
	{
	}

	SimulateStatus& operator=(const SimulateStatus& other)
	{
		// 检查自赋值
		if (this != &other)
		{
			platformSize = other.platformSize;
			objectRange = other.objectRange;
			toolPos = other.toolPos;
			wcsAnchor = other.wcsAnchor;
			zoom = other.zoom;
			Zup = other.Zup;
			Zdown = other.Zdown;
			ncstep = other.ncstep;
		}
		return *this;
	}

	void SetToolRadius(PLC_TYPE_LREAL radius);
	PLC_TYPE_LREAL GetToolRadius();

	void SetToolDistance(PLC_TYPE_LREAL distance);
	PLC_TYPE_LREAL GetToolDistance();
private:
	std::mutex mutex;
};

struct GCodeRecord
{
public:
	GCodeRecord(const std::string& _s, CNCSYS::EntityVGPU* _ae, int _index, const glm::mat4& _transform, int _row)
		:content(_s), attachedEntity(_ae), transformation(_transform), sampleIndex(_index), row(_row)
	{

	}
	std::string content;
	CNCSYS::EntityVGPU* attachedEntity = nullptr;
	Path2D* attachedPath = nullptr;
	int sampleIndex;
	glm::mat4 transformation;
	int row;
};

struct AuthInfo
{
	std::string chipID;
	std::string pcUUID;
	std::string authCode;
	std::string authTime;

	int limitYear = 999;
	int limitMonth = 12;
	int limitDay = 31;
};

extern SimulateStatus g_MScontext;
extern std::string g_writeNcFileFtpPath;
extern std::string g_ftpDir;
extern MainLayer* g_mainWindow;
extern QSettings* g_settings;
extern QString g_plcSearchRootNode;
extern QString g_plcUrl;
extern QString g_plcVarCnfigExcelUrl;
extern std::unordered_map<std::string, std::string> g_ConfigableKeys;
extern std::vector<std::string> g_preRegKeys;
extern D8 g_dogKey;
extern AuthInfo g_authInfo;
extern char DevicePath[MAX_PATH];
extern GeomDirection g_defaultDir;
extern GeomDirection g_defaultDirReverse;
extern std::shared_ptr<spdlog::logger> g_file_logger;

void InitPLConfig();
void InitLogger();
int InitProgressContext();
int CheckAuth(bool popUpMsg = true);
void AfterInitProgressContext();


#define LOG_FILE_INFO(...) g_file_logger->info(__VA_ARGS__)
#define LOG_FILE_ERROR(...) g_file_logger->error(__VA_ARGS__)
#define LOG_FILE_WARN(...) g_file_logger->warn(__VA_ARGS__)