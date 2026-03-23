#include "UI/OverallWindow.h"
#include "Graphics/Sketch.h"
#include "Common/Program.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "NetWork/OPClient.h"
#include "NetWork/FtpClient.h"
#include "Graphics/Primitives.h"
#include <Controls/ScadaMessageHandler.h>
#include "UI/GCodeEditor.h"
#include "UI/Components/HmiTemplateWebViewer.h"
#include "UI/Components/HmiTemplateMsgBox.h"
#include "UI/TaskListWindow.h"
#include "Graphics/Anchor.h"
#include "ModalEvent/EvCanvasSetNewScene.h"
#include "Common/Program.h"
#include <QApplication>
#include <chrono> 
#include <QVBoxLayout>
#include <QMessageBox>
#include <random>

using namespace CNCSYS;

void executeAfterDelay(std::chrono::milliseconds delay, std::function<void()> task) {
	std::thread t([delay, task]() {
		std::this_thread::sleep_for(delay);
		task();
		});
	t.detach();
}
OverallWindow::OverallWindow()
{
	setWindowFlags(Qt::FramelessWindowHint);
	this->setWindowIcon(QIcon(ICOPATH(logo.ico)));

	g_opcuaClient->reconnectCallback = [&]()
		{
			HmiTemplateWebViewer::Reload();
		};

	mainWindow = new MainLayer(this);
	//menu = new MenuLayerTop(this);
	QVBoxLayout* hbox = new QVBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	//hbox->addWidget(menu);
	hbox->addWidget(mainWindow);
	this->setLayout(hbox);
	this->showMaximized();

	//整个下位机通信交互逻辑在这里
	monitorIndexPage = new ScadaNode();
	monitorIndexPage->BindParam(g_ConfigableKeys["IndexTargetMainArea"]);
	monitorIndexPage->updateCallback = [&]()
		{
			PLC_TYPE_INT curVal = monitorIndexPage->GetInt();

			PLC_TYPE_INT SubNavIndex;
			ReadPLC_OPCUA(g_ConfigableKeys["IndexSubArea"].c_str(), &SubNavIndex, AtomicVarType::INT);

			if (curVal == 1 && SubNavIndex == 1)
			{
				executeAfterDelay(std::chrono::milliseconds(200), [this]() {
					// 确保在主线程调用 SetShow
					QMetaObject::invokeMethod(this, "SetShow", Qt::QueuedConnection);
					});
			}
			else
			{
				QMetaObject::invokeMethod(this, "SetHide", Qt::QueuedConnection);
			}
			monitorIndexPage->lastValue.lastInt = curVal;
		};

	monitorUploadFTP = new ScadaNode();
	monitorUploadFTP->BindParam(g_ConfigableKeys["PCFileFTP"]);
	monitorUploadFTP->updateCallback = [&]()
		{
			static std::array<bool, 10> oldRequest = g_stationPCFileFTP;
			bool ifRequest = false;
			for (int i = 0; i < stationSize;i++)
			{
				if (g_stationPCFileFTP[i] && !oldRequest[i])
				{
 					std::string ftpFilePath = "../FTP/" + extractFTPFileName(QString::fromLocal8Bit(g_mainWindow->sketchLists[i+1]->source.c_str()));
					PLC_TYPE_STRING newSliceFileName = (PLC_TYPE_STRING)malloc(256);
					strcpy_s(newSliceFileName, ftpFilePath.length() + 1, ftpFilePath.c_str());

					WritePLC_OPCUA(g_ConfigableKeys[QString("WorkFileStation%1").arg(i).toStdString()].c_str(), (void*)ftpFilePath.c_str(), AtomicVarType::STRING);

					free(newSliceFileName);
					ifRequest = true;
				}
				oldRequest[i] = g_stationPCFileFTP[i];
			}
			if(ifRequest)
			{
				Anchor::GetInstance()->CleanCache();
	
				std::array<bool, 10>* writeArray = static_cast<std::array<bool, 10>*>(g_writePersistence[g_ConfigableKeys["PCFileFTPDone"].c_str()]);
				for (int i = 0; i < stationSize; i++)
				{
					if (writeArray)
					{
						writeArray->data()[i] = TRUE;
					}
				}
				WritePLC_OPCUA(g_ConfigableKeys["PCFileFTPDone"].c_str(), nullptr, AtomicVarType::ARRAY_BOOL);
				ifRequest = false;
			}
		};

	monitorHeartBeat = new ScadaNode();
	monitorHeartBeat->BindParam(g_ConfigableKeys["HeartbeatCount"]);
	monitorHeartBeat->SetUpdateRate(5 * 1000);
	monitorHeartBeat->updateCallback = [&]()
		{
			PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[monitorHeartBeat->bindTag]);
			static auto now = std::chrono::system_clock::now();
			static auto lastUpdateTime = std::chrono::system_clock::now();
			if (plcInfo)
			{
				now = std::chrono::system_clock::now();
				auto duration = now - lastUpdateTime;
				auto durationInMilisec = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
				static PLC_TYPE_DWORD enterCount = 1;

				if (durationInMilisec > plcInfo->collectionInterval)
				{
					PLC_TYPE_DWORD curVal = monitorHeartBeat->GetDword();
					if (curVal == monitorHeartBeat->lastValue.lastInt)
					{
						QMetaObject::invokeMethod(ScadaMessageHandler::GetInstance(), "handleOpcDisconnected",
							Qt::QueuedConnection,
							Q_ARG(OPClient*, ScadaScheduler::GetInstance()->GetOPCommClient()));
					}
					monitorHeartBeat->lastValue.lastInt = curVal;
					lastUpdateTime = now;

					//WritePLC_OPCUA(g_ConfigableKeys["HeartbeatCountPC"].c_str(),&enterCount,AtomicVarType::DWORD);
					//enterCount++;
				}
			}
		};
	if (g_settings->value("Settings/Mode") != "Debug")
	{
		mainWindow->GetCanvasPanel()->hide();
	}


	monitorToolRadius = new ScadaNode();
	monitorToolRadius->BindParam(g_ConfigableKeys["ToolRadius"]);
	monitorToolRadius->lastValue.lastLReal = 0.0f;
	monitorToolRadius->updateCallback = [&]()
	{
			PLC_TYPE_LREAL radius = monitorToolRadius->GetLreal();
			if (!(radius == monitorToolRadius->lastValue.lastLReal))
			{
				g_MScontext.SetToolRadius(radius);
				monitorToolRadius->lastValue.lastLReal = radius;
			}
	};

	monitorToolDistance = new ScadaNode();
	monitorToolDistance->BindParam(g_ConfigableKeys["RemainDistance"]);
	monitorToolDistance->lastValue.lastLReal = 0.0f;
	monitorToolDistance->updateCallback = [&]()
	{
			PLC_TYPE_LREAL distance = monitorToolDistance->GetLreal();
			if (!(distance == monitorToolDistance->lastValue.lastLReal))
			{
				g_MScontext.SetToolDistance(distance);
				monitorToolDistance->lastValue.lastLReal = distance;
			}
	};

	monitorAutoBusy = new ScadaNode();
	monitorAutoBusy->BindParam(g_ConfigableKeys["AutoBusy"]);
	monitorAutoBusy->lastValue.lastBool = false;
	monitorAutoBusy->updateCallback = [&]()
	{
		PLC_TYPE_BOOL startCNC = monitorAutoBusy->GetBool();
		if (!(startCNC == monitorAutoBusy->lastValue.lastBool))
		{
			if (startCNC)
			{
				Anchor::GetInstance()->CleanCache();
			}
			monitorAutoBusy->lastValue.lastBool = startCNC;
		}

	};

	monitorStationIndex = new ScadaNode();
	monitorStationIndex->BindParam(g_ConfigableKeys["StationIndex"]);
	monitorStationIndex->lastValue.lastInt = -1;
	monitorStationIndex->updateCallback = [&]()
	{
		PLC_TYPE_INT currentStationId = monitorStationIndex->GetInt();
		if (!(currentStationId == monitorStationIndex->lastValue.lastInt))
		{
			Anchor::GetInstance()->CleanCache();
			monitorStationIndex->lastValue.lastInt = currentStationId;
			bool ok = QMetaObject::invokeMethod(
				g_mainWindow->stationTab,
				"setCurrentIndex",
				Qt::QueuedConnection,
				Q_ARG(int, currentStationId+1)
			);
		}
	};

	monitorClearBuffer = new ScadaNode();
	monitorClearBuffer->BindParam(g_ConfigableKeys["AnimatorCacheClear"]);
	monitorClearBuffer->updateCallback = [&]()
	{
		bool ifReset = false;
		for (int i = 0; i < stationSize;i++)
		{
			if (g_stationClearPC[i])
			{
				Anchor::GetInstance()->CleanCache();
				ifReset = true;
			}
		}
		if (ifReset)
		{
			std::array<bool, 10>* writeArray = static_cast<std::array<bool, 10>*>(g_writePersistence[g_ConfigableKeys["AnimatorCacheClear"].c_str()]);
			for (int i = 0; i < stationSize; i++)
			{
				if (writeArray)
				{
					writeArray->data()[i] = false;
				}
			}
			WritePLC_OPCUA(g_ConfigableKeys["AnimatorCacheClear"].c_str(), nullptr, AtomicVarType::ARRAY_BOOL);
		}
	};

	//初始清理所有历史文件
	PLC_TYPE_STRING empty = (PLC_TYPE_STRING)malloc(1);
	empty[0] = '\0';
	WritePLC_OPCUA(g_ConfigableKeys["WorkFileStation0"].c_str(), (void*)empty, AtomicVarType::STRING);
	WritePLC_OPCUA(g_ConfigableKeys["WorkFileStation1"].c_str(), (void*)empty, AtomicVarType::STRING);
	WritePLC_OPCUA(g_ConfigableKeys["WorkFileStation2"].c_str(), (void*)empty, AtomicVarType::STRING);
	WritePLC_OPCUA(g_ConfigableKeys["WorkFileStation3"].c_str(), (void*)empty, AtomicVarType::STRING);
	WritePLC_OPCUA(g_ConfigableKeys["WorkFileStation4"].c_str(), (void*)empty, AtomicVarType::STRING);

	ScadaScheduler::GetInstance()->AddNode(monitorIndexPage);
	ScadaScheduler::GetInstance()->AddNode(monitorUploadFTP);
	ScadaScheduler::GetInstance()->AddNode(monitorHeartBeat);
	ScadaScheduler::GetInstance()->AddNode(monitorToolRadius);
	ScadaScheduler::GetInstance()->AddNode(monitorToolDistance);
	ScadaScheduler::GetInstance()->AddNode(monitorAutoBusy);
	ScadaScheduler::GetInstance()->AddNode(monitorStationIndex);
	ScadaScheduler::GetInstance()->AddNode(monitorClearBuffer);
	ScadaScheduler::GetInstance()->RegisterReadBackVarKey(g_ConfigableKeys["AnimatorCycleTime"]);


}

OverallWindow::~OverallWindow()
{

}
void OverallWindow::closeEvent(QCloseEvent* event)
{
	g_file_logger->flush();
}
void OverallWindow::SetShow()
{
	mainWindow->GetCanvasPanel()->show();
}

void OverallWindow::SetHide()
{
	if (g_settings->value("Settings/Mode") != "Debug")
	{
		mainWindow->GetCanvasPanel()->hide();
	}
}