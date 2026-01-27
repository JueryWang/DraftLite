#include "UI/OverallWindow.h"
#include "Graphics/Sketch.h"
#include "Common/ProgressInfo.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "NetWork/OPClient.h"
#include "Graphics/Primitives.h"
#include <Controls/ScadaMessageHandler.h>
#include "UI/GCodeEditor.h"
#include "UI/Components/HmiTemplateWebViewer.h"
#include "UI/Components/HmiTemplateMsgBox.h"
#include "UI/TaskListWindow.h"
#include "ModalEvent/EvCanvasSetNewScene.h"
#include "Common/ProgressInfo.h"
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
	SketchGPU* sketch = new SketchGPU();
	setWindowFlags(Qt::FramelessWindowHint);
	this->setWindowIcon(QIcon(ICOPATH(logo.ico)));

	g_opcuaClient->reconnectCallback = [&]()
		{
			HmiTemplateWebViewer::Reload();
		};

	mainWindow = new MainLayer(sketch, this);
	//menu = new MenuLayerTop(this);
	QVBoxLayout* hbox = new QVBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	//hbox->addWidget(menu);
	hbox->addWidget(mainWindow);
	this->setLayout(hbox);
	this->showMaximized();
	monitorIndexPage = new ScadaNode();
	monitorIndexPage->BindParam(g_ConfigableKeys["IndexTargetMainArea"]);
	monitorIndexPage->updateCallback = [&]()
		{
			PLC_TYPE_INT curVal = monitorIndexPage->GetInt();

			PLC_TYPE_INT SubNavIndex;
			ReadPLC_OPCUA(g_ConfigableKeys["IndexSubArea"].c_str(), &SubNavIndex, AtomicVarType::INT);

			if (curVal == 3 && SubNavIndex == 1)
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
			PLC_TYPE_BOOL ifRequestFile = monitorUploadFTP->GetBool();
			if (ifRequestFile && !(ifRequestFile == monitorUploadFTP->lastValue.lastBool))
			{
				static TaskListWindow* taskList = TaskListWindow::GetInstance();
				if (taskList->items.size() > 0)
				{
					std::string ftpFilePath = "../FTP/" + taskList->items[taskList->currentRequestNumber]->attachedWidget->ftpUploadSource;
					PLC_TYPE_STRING newSliceFileName = (PLC_TYPE_STRING)malloc(256);
					strcpy_s(newSliceFileName, ftpFilePath.length() + 1, ftpFilePath.c_str());
					WritePLC_OPCUA(g_ConfigableKeys["WorkFileName"].c_str(), (void*)ftpFilePath.c_str(), AtomicVarType::STRING);
					ToDoListItemWidget* widget = taskList->CurrentItemWidget();
					taskList->taskLists->setCurrentRow(taskList->currentRequestNumber);
					EvCanvasSetNewScene* evSetScene = new EvCanvasSetNewScene(widget->sketch, widget->ocsSys);
					QApplication::postEvent(g_canvasInstance->GetFrontWidget(), evSetScene, Qt::HighEventPriority);
					free(newSliceFileName);
					taskList->CurrentItemWidget()->AddCounter();
					taskList->currentRequestNumber = (taskList->currentRequestNumber + 1) % (taskList->items.size());
				}
				PLC_TYPE_BOOL uploadDone = true;
				WritePLC_OPCUA(g_ConfigableKeys["PCFileFTPDone"].c_str(), &uploadDone, AtomicVarType::BOOL);
			}
			monitorUploadFTP->lastValue.lastBool = ifRequestFile;
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

				if (durationInMilisec > plcInfo->collectionInterval)
				{
					PLC_TYPE_INT curVal = monitorHeartBeat->GetInt();
					if (curVal == monitorHeartBeat->lastValue.lastInt)
					{
						QMetaObject::invokeMethod(ScadaMessageHandler::GetInstance(), "handleOpcDisconnected",
							Qt::QueuedConnection,
							Q_ARG(OPClient*, ScadaScheduler::GetInstance()->GetOPCommClient()));
					}
					monitorHeartBeat->lastValue.lastInt = curVal;
					lastUpdateTime = now;
				}
			}
		};

	monitorAutoBusy = new ScadaNode();
	monitorAutoBusy->BindParam(g_ConfigableKeys["AutoBusy"]);
	monitorAutoBusy->lastValue.lastBool = false;
	monitorAutoBusy->updateCallback = [&]()
		{
			static TaskListWindow* taskList = TaskListWindow::GetInstance();
			PLC_TYPE_BOOL busy = monitorAutoBusy->GetBool();
			if (!(busy == monitorAutoBusy->lastValue.lastBool))
			{
				if (busy)
				{
					taskList->toolBar->DisableTools();
				}
				else
				{
					taskList->toolBar->EnableTools();
				}
			}
			monitorAutoBusy->lastValue.lastBool = busy;
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

	ScadaScheduler::GetInstance()->AddNode(monitorIndexPage);
	ScadaScheduler::GetInstance()->AddNode(monitorUploadFTP);
	ScadaScheduler::GetInstance()->AddNode(monitorHeartBeat);
	ScadaScheduler::GetInstance()->AddNode(monitorAutoBusy);
	ScadaScheduler::GetInstance()->AddNode(monitorToolRadius);
	ScadaScheduler::GetInstance()->AddNode(monitorToolDistance);
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