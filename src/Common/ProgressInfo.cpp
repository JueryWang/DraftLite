#include <QDir>
#include "IO/ExcelProcessor.h"
#include "Auth/WG_Authorization.h"
#include "Auth/CounterDownManager.h"
#include "Auth/KeyFuncs.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "Controls/ScadaMessageHandler.h"
#include "Controls/ScadaScheduler.h"
#include "Common/ProgressInfo.h"
#include "NetWork/FtpClient.h"
#include "NetWork/OPClient.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <QMessageBox>
#include <QPushButton>
#include <QClipBoard>
#include <QGuiApplication>

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
std::map<std::string, std::string> g_VarNodePathDict;
std::map<std::string, std::string> g_ConfigableKeys;
std::string g_ftpDir = "Share_files_anonymity";
D8 g_dogKey;
AuthInfo g_authInfo;

void InitPLConfig()
{
	g_VarNodePathDict =
	{
		{"ActVelocityCNC","gvlHMI.stStatusGearChamferMachine.stStatusCADWork.fActVelocityCNC"},
		{"CurrentRowCNC","gvlHMI.stStatusGearChamferMachine.stStatusCADWork.iCurrentRowCNC"},
		{"IndexTargetMainArea","gvlHMI.stStatusGearChamferMachine.iIndexTargetMainArea"},			//网页往前分页索引
		{"PChmiCheck","gvlHMI.stStatusGearChamferMachine.xPChmiCheck"},
		{"PChmiReady","gvlHMI.stCommandGearChamferMachine.xPChmiReady"},
		{"PositionAxisX","gvlHMI.stStatusGearChamferMachine.stCoordAxis.fPositionAxisX"},
		{"PositionAxisY","gvlHMI.stStatusGearChamferMachine.stCoordAxis.fPositionAxisY"},
		{"WorkFileName","gvlHMI.stParameterGearChamferMachine.stParameterCADWork.sWorkFileName"},	//当前G代码文件名
		{"PCFileFTPDone","gvlHMI.xPCFileFTPDone"},								//PC相应G代码上传FTP完成
		{"PCFileFTP","gvlHMI.stIOGearChamferMachine.xPCFileFTP"},				//PLC请求G代码上传FTP
		{"HeartbeatCount","gvlHMI.udiHeartbeatCount"},							//心跳包检测
		{"IndexSubArea","gvlHMI.stCommandGearChamferMachine.iIndexSubArea"},	//绘图区显示信号
		{"PageInit","gvlHMI.stCommandGearChamferMachine.xPageInit"},			//初始化界面
		{"ChangeSlice","gvlHMI.stIOGearChamferMachine.xPCChange"},				//PLC请求多计划切图
		{"ChangeSliceDone","gvlHMI.xPCChangeDone"},								//切图PC完成信号
		{"AutoBusy","gvlHMI.stStatusGearChamferMachine.stStatusCADWork.xAutoBusy"}   //设备自动运行中
	};

	g_settings = new QSettings(QDir::currentPath() + "/config.ini", QSettings::IniFormat);

	screen_resolution_x = g_settings->value("Screen/Width").toInt();
	screen_resolution_y = g_settings->value("Screen/Height").toInt();
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

	g_settings->beginGroup("ConfigableKeys");
	QStringList allKeys = g_settings->allKeys();
	for (const QString& key : allKeys)
	{
		QVariant value = g_settings->value(key);
		std::string _key = key.toStdString();
		std::string _value = value.toString().toStdString();
		g_ConfigableKeys[_key] = _value;
		//g_ConfigableKeys.insert(key.toStdString(), value.toString().toStdString());
	}
	g_settings->endGroup();

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
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	if (g_settings->value("Settings/Debug") == "Debug")
	{
		console_sink->set_level(spdlog::level::debug);
	}
	console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logger.log", true);
	file_sink->set_level(spdlog::level::warn);

	std::vector<spdlog::sink_ptr> sinks{ console_sink,file_sink };
	auto logger = std::make_shared<spdlog::logger>("network_logger", sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::debug);  // 全局最低级别
	spdlog::register_logger(logger);
}

int InitProgressContext()
{
	bool success = CheckAuth();
	if (!success)
	{
		CounterDownManager* counterDown = new CounterDownManager();
	}
	InitPLConfig();
	InitLogger();
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


