#include <QDir>
#include "IO/ExcelProcessor.h"
#include "Auth/WG_Authorization.h"
#include "Auth/CounterDownManager.h"
#include "Auth/KeyFuncs.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "Controls/ScadaMessageHandler.h"
#include "Controls/ScadaScheduler.h"
#include "Common/Program.h"
#include "NetWork/FtpClient.h"
#include "NetWork/OPClient.h"
#include <unordered_map>
#include <QMessageBox>
#include <QPushButton>
#include <QClipBoard>
#include <QGuiApplication>
#include <fstream>

using namespace CNCSYS;
char DevicePath[MAX_PATH];

#pragma pack(1)

SimulateStatus g_MScontext;
std::string g_writeNcFileFtpPath;
MainLayer* g_mainWindow = nullptr;
QSettings* g_settings;
QString g_plcUrl;
QString g_plcVarCnfigExcelUrl;
QString g_plcSearchRootNode;
std::vector<std::string> g_preRegKeys;
std::unordered_map<std::string, std::string> g_ConfigableKeys;
std::string g_ftpDir = "Share_files_anonymity";
GeomDirection g_defaultDir = GeomDirection::CCW;
GeomDirection g_defaultDirReverse = GeomDirection::CW;
D8 g_dogKey;
AuthInfo g_authInfo;
std::shared_ptr<spdlog::logger> g_file_logger;

SYSTEMTIME CurTime, LimitTimer;

int CompareSystemTimes(const SYSTEMTIME& st1, const SYSTEMTIME& st2);

void InitPLConfig()
{
	g_ConfigableKeys =
	{
		{"CurrentRowCNC","gvlHMI.stStatusCADWork.iCurrentRowCNC"},
		{"IndexTargetMainArea","gvlHMI.stStatusGearChamferMachine.iIndexTargetMainArea"},			//网页往前分页索引
		{"PChmiCheck","gvlHMI.stStatusGearChamferMachine.xPChmiCheck"},
		{"PChmiReady","gvlHMI.stCommandGearChamferMachine.xPChmiReady"},
		{"WorkFileStation0","gvlHMI.stParameterGearChamferMachine.astParameterCADWork[0].sWorkFileName"},	//工位0文件名
		{"WorkFileStation1","gvlHMI.stParameterGearChamferMachine.astParameterCADWork[1].sWorkFileName"},	//工位1文件名
		{"WorkFileStation2","gvlHMI.stParameterGearChamferMachine.astParameterCADWork[2].sWorkFileName"},	//工位2文件名
		{"WorkFileStation3","gvlHMI.stParameterGearChamferMachine.astParameterCADWork[3].sWorkFileName"},	//工位4文件名
		{"WorkFileStation4","gvlHMI.stParameterGearChamferMachine.astParameterCADWork[4].sWorkFileName"},	//工位5文件名
		{"PCFileFTPDone","gvlGlobalData.stIOGearChamferMachine.axPCFileFTPDone"},								//PC相应G代码上传FTP完成
		{"PCFileFTP","gvlGlobalData.stIOGearChamferMachine.axPCFileFTP"},				//PLC请求G代码上传FTP
		{"HeartbeatCount","gvlHMI.udiHeartbeatCount"},							//心跳包检测
		{"HeartbeatCountPC","gvlHMI.udiHeartbeatCountPC"},
		{"IndexSubArea","gvlHMI.stCommandGearChamferMachine.iIndexSubArea"},	//绘图区显示信号
		{"PageInit","gvlHMI.stCommandGearChamferMachine.xPageInit"},			//初始化界面
		{"ChangeSlice","gvlGlobalData.stIOGearChamferMachine.axPCChange"},				//PLC请求多计划切图
		{"ChangeSliceDone","gvlGlobalData.stIOGearChamferMachine.axPCChangeDone"},		//切图PC完成信号
		{"AutoBusy","gvlHMI.stStatusCADWork.xAutoBusy"},   //设备自动运行中
		{"ToolRadius","gvlHMI.stParameterCADWork.stParaNCInterpreter.fToolRadius"},
		{"AutoStart","gvlHMI.stCommandGearChamferMachine.xAutoStart"},
		{"AutoPause","gvlHMI.stStatusCADWork.xAutoPause"},
		//{"RemainDistance","gvlHMI.stParameterGearChamferMachine.fRemainDistance"},
		{"AnimatorBufferLengthQueueA","gvlHMI.stCNCVisualCADWork.iIndexCNCVisualA"},
		{"AnimatorBufferQueueA","gvlHMI.stCNCVisualCADWork.astCNCQueueA"},
		{"AnimatorBufferLengthQueueB","gvlHMI.stCNCVisualCADWork.iIndexCNCVisualB"},
		{"AnimatorBufferQueueB","gvlHMI.stCNCVisualCADWork.astCNCQueueB"},
		{"AnimatorCycleTime","gvlHMI.stConfigGearChamferMachine.dwIpoCycle"},
		{"AnimatorCacheClear","gvlGlobalData.stIOGearChamferMachine.axClearPC"},
		{"StationIndex","gvlHMI.stCommandGearChamferMachine.iIndexStation"}
	};

	g_settings = new QSettings(QDir::currentPath() + "/config.ini", QSettings::IniFormat);

	screen_resolution_x = g_settings->value("Screen/Width").toInt();
	screen_resolution_y = g_settings->value("Screen/Height").toInt();
	canvasAnchorX = g_settings->value("Screen/CanvasAnchor.X").toInt();
	canvasAnchorY = g_settings->value("Screen/CanvasAnchor.Y").toInt();
	CANVAS_WIDTH = g_settings->value("Screen/CanvasWidth").toInt();
	CANVAS_HEIGHT = g_settings->value("Screen/CanvasHeight").toInt();
	stationSize = g_settings->value("Settings/StationSize").toInt();

	if (g_settings->value("Layout/DrawPanelWidthFactor").toFloat() != 0)
	{
		canvas_panel_width_ratio = g_settings->value("Layout/DrawPanelWidthFactor").toFloat();
	}
	if (g_settings->value("Layout/DrawPanelWidthFactor").toFloat() != 0)
	{
		gcode_panel_width_ratio = g_settings->value("Layout/DrawPanelWidthFactor").toFloat();
	}
	if (g_settings->value("Layout/GCodeEditorHeightFactor").toFloat() != 0)
	{
		gcode_panel_height_ratio = g_settings->value("Layout/GCodeEditorHeightFactor").toFloat();
	}
	if (g_settings->value("Layout/DrawPanelHeightFactor").toFloat() != 0)
	{
		canvas_panel_height_ratio = g_settings->value("Layout/DrawPanelHeightFactor").toFloat();
	}
	if (g_settings->value("Layout/BlankHeightFactor").toFloat() != 0)
	{
		blank_height_ratio = g_settings->value("Layout/BlankHeightFactor").toFloat();
	}
	if (g_settings->value("Layout/TaskWindowPanelWidthFactor").toFloat() != 0)
	{
		taskpanel_width_ratio = g_settings->value("Layout/TaskWindowPanelWidthFactor").toFloat();
	}
	if (g_settings->value("Layout/TaskWindowPanelHeightFactor").toFloat() != 0)
	{
		taskpanel_height_ratio = g_settings->value("Layout/TaskWindowPanelHeightFactor").toFloat() / blank_height_ratio;
	}
	if (g_settings->value("Layout/ToDoCanvasWidthFactor").toFloat() != 0)
	{
		todoCanvas_width_ratio = g_settings->value("Layout/ToDoCanvasWidthFactor").toFloat();
	}
	if (g_settings->value("Layout/ToDoCanvasHeightFactor").toFloat() != 0)
	{
		todoCanvas_height_ratio = g_settings->value("Layout/ToDoCanvasHeightFactor").toFloat();
	}
	g_plcUrl = g_settings->value("PLC/URL").toString();
	g_plcVarCnfigExcelUrl = g_settings->value("PLC/VarConfig").toString();
	g_plcSearchRootNode = g_settings->value("PLC/ParseVarRoot").toString();

	g_plcUrl = g_settings->value("PLC/URL").toString();
	g_plcVarCnfigExcelUrl = g_settings->value("PLC/VarConfig").toString();
	g_plcSearchRootNode = g_settings->value("PLC/ParseVarRoot").toString();

	//g_settings->beginGroup("ConfigableKeys");
	//QStringList allKeys = g_settings->allKeys();
	//for (const QString& key : allKeys)
	//{
	//	QVariant value = g_settings->value(key);
	//	std::string _key = key.toStdString();
	//	std::string _value = value.toString().toStdString();
	//	g_ConfigableKeys[_key] = _value;
	//	//g_ConfigableKeys.insert(key.toStdString(), value.toString().toStdString());
	//}
	//g_settings->endGroup();

	ExcelProcessor reader;
	g_opcuaClient = new OPClient();
	if (g_opcuaClient)
	{
		reader.ReadPLCVariantMap();
	}
	g_opcuaClient->ConnectToServer(g_plcUrl.toStdString().c_str());
	ScadaScheduler::GetInstance()->SetOPCommClient(g_opcuaClient);
}

void InitLogger()
{
	g_file_logger = spdlog::basic_logger_mt("global_file_logger", "./logger.log", true);
	g_file_logger->set_level(spdlog::level::trace);
	// - 方式1：warn及以上级别立即flush（推荐）
	g_file_logger->flush_on(spdlog::level::warn);
	// - 方式2：也可设置每1秒自动flush（兜底）
	spdlog::flush_every(std::chrono::seconds(1));
	// 配置日志格式
	g_file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
	// 设置日志级别
	g_file_logger->set_level(spdlog::level::trace);
	// 错误级别立即刷新
	g_file_logger->flush_on(spdlog::level::err);
}

int InitProgressContext()
{
	InitLogger();
	QDir tempDir("./temp");
	// 递归删除整个 temp 目录（包括所有内容和目录本身）
	tempDir.removeRecursively();
	QDir currentDir = QDir::current();
	currentDir.mkdir("temp");

	bool success = CheckAuth();
	if (!success)
	{
		CounterDownManager* counterDown = new CounterDownManager();
	}
	InitPLConfig();
	//{
	FtpClient::SetFtpUrl("ftp://192.168.6.6:21");
	FtpClient::CleanRemoteDirectory(g_ftpDir);
	g_MScontext.platformSize = AABB(glm::vec3(0.0f), glm::vec3(300.0f, 500.0f, 0.0f));
	g_MScontext.toolPos = glm::vec3(0.0f);
	g_MScontext.wcsAnchor = glm::vec3(0.0, 0.0, 0.0);
	g_MScontext.Zup = 3.0f;
	g_MScontext.Zdown = -1.0f;
	g_MScontext.velocity = 50.0f;
	g_MScontext.acceleration = 1000.0f;
	g_MScontext.deceleartion = -1000.0f;
	//}

	return 0;
}

int CheckAuth(bool popUpMsg)
{
	g_authInfo.chipID = GetAuthChipId();
	g_authInfo.pcUUID = GetAuthPCUUID();
	g_authInfo.authCode = GetAuthVeriCode();
	g_authInfo.authTime = GetAuthTime();

	bool ret = true;
	if (g_dogKey.FindPort(0, DevicePath))
	{
		ret = false;
		if (popUpMsg)
		{
			QMessageBox::critical(NULL, QStringLiteral("错误"), QStringLiteral("未找到加密锁,请插入加密锁后，再进行操作."));
		}
		return ret;
	}

	std::wstring cmd = L"powershell -Command \"Get-WmiObject Win32_ComputerSystemProduct | Select-Object -ExpandProperty UUID\""; // 目标 cmd 命令
	std::string output;
	std::string error;

	ExecuteCmdWithoutConsole(cmd, output, error);
	std::vector<std::string> lines = SplitByCRLF(output);
	std::string localUUID = "";
	if (lines.size() > 0)
	{
		localUUID = lines[0];
	}

	char localChipId[33];
	g_dogKey.GetChipID(localChipId, DevicePath);
	int year;
	BYTE month;
	BYTE day;
	g_dogKey.GetLimitDate(&year, &month, &day, DevicePath);
	g_authInfo.limitYear = year;
	g_authInfo.limitMonth = month;
	g_authInfo.limitDay = day;

	LimitTimer.wYear = g_authInfo.limitYear;
	LimitTimer.wMonth = g_authInfo.limitMonth;
	LimitTimer.wDay = g_authInfo.limitDay;
	::GetSystemTime(&CurTime);

	if (CompareSystemTimes(CurTime,LimitTimer) >= 0)
	{
		QMessageBox::critical(NULL, QStringLiteral("错误"), QStringLiteral("加密狗许可证过期,请重新授权."));
		return false;
	}

	char* originChipId = GetAuthChipId();
	if (strcmp(localChipId, originChipId) != 0)
	{
		ret = false;
		auto messageDif = GetKeyMatchDifference(localChipId, (char*)localUUID.c_str());
		QMessageBox msgBox;
		msgBox.setWindowTitle(QStringLiteral("错误"));
		msgBox.setText(QString("加密狗ID与原始下载不同,重新下载后，再进行操作.\n具体信息:%1").arg(messageDif));
		msgBox.setIcon(QMessageBox::Critical);
		QPushButton* copyMsgBtn = msgBox.addButton("拷贝信息", QMessageBox::ApplyRole);
		msgBox.addButton("关闭", QMessageBox::ApplyRole);
		QObject::connect(copyMsgBtn, &QPushButton::clicked, [&]()
			{
				QClipboard* clipboard = QGuiApplication::clipboard();

				if (clipboard)
				{
					clipboard->setText(messageDif);
				}
			});
		if (popUpMsg)
		{
			msgBox.exec();
		}
		return ret;
	}
	if (localUUID != g_authInfo.pcUUID)
	{
		ret = false;
		QString messageDif = GetKeyMatchDifference(localChipId, (char*)localUUID.c_str());
		QMessageBox msgBox;
		msgBox.setWindowTitle(QStringLiteral("错误"));
		msgBox.setText(QString("主机与原始授权不同,重新下载后，再进行操作.\n具体信息:%1").arg(messageDif));
		msgBox.setIcon(QMessageBox::Critical);
		QPushButton* copyMsgBtn = msgBox.addButton("拷贝信息", QMessageBox::ApplyRole);
		msgBox.addButton("关闭", QMessageBox::ApplyRole);
		QObject::connect(copyMsgBtn, &QPushButton::clicked, [&]()
			{
				QClipboard* clipboard = QGuiApplication::clipboard();

				if (clipboard)
				{
					clipboard->setText(messageDif);
				}
			});
		if (popUpMsg)
		{
			msgBox.exec();
		}
		ScadaMessageHandler::GetInstance()->handleAuthNotMatch(nullptr, g_authInfo.chipID, std::string(localChipId), g_authInfo.pcUUID, localUUID);
		return ret;
	}

	return ret;
}

void AfterInitProgressContext()
{
	ScadaMessageHandler::GetInstance()->handleResoreHistory();
}

void SimulateStatus::SetToolRadius(PLC_TYPE_LREAL radius)
{
	std::lock_guard<std::mutex> lock(mutex);
	toolRadius = radius;
}

PLC_TYPE_LREAL SimulateStatus::GetToolRadius()
{
	std::lock_guard<std::mutex> lock(mutex);
	return toolRadius;
}

void SimulateStatus::SetToolDistance(PLC_TYPE_LREAL distance)
{
	std::lock_guard<std::mutex> lock(mutex);
	toolDistance = distance;
}

PLC_TYPE_LREAL SimulateStatus::GetToolDistance()
{
	std::lock_guard<std::mutex> lock(mutex);
	return toolDistance;
}


int CompareSystemTimes(const SYSTEMTIME& st1, const SYSTEMTIME& st2) {
	FILETIME ft1, ft2;
	SystemTimeToFileTime(&st1, &ft1);
	SystemTimeToFileTime(&st2, &ft2);

	// 返回值含义：
	// -1: ft1 < ft2 (st1 早于 st2)
	//  0: ft1 == ft2
	//  1: ft1 > ft2 (st1 晚于 st2)
	return CompareFileTime(&ft1, &ft2);
}