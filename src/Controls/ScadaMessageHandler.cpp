#include "Controls/ScadaMessageHandler.h"
#include "Controls/ScadaScheduler.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/MainLayer.h"
#include "NetWork/OPClient.h"
#include "Common/Program.h"
#include "IO/Database.h"
#include "IO/DxfProcessor.h"
#include <QGridLayout>
#include <QApplication>
#include <functional>
#include <QMessageBox>
#include <QLineEdit>

ScadaMessageHandler* ScadaMessageHandler::instance = nullptr;
std::mutex ScadaMessageHandler::mtx;
using namespace CNCSYS;

ScadaMessageHandler* ScadaMessageHandler::GetInstance()
{
	if (instance == nullptr){
		std::lock_guard<std::mutex> lock(mtx);
		if (instance == nullptr)
		{
			instance = new ScadaMessageHandler();
		}
	}
	return instance;
}

ScadaMessageHandler::ScadaMessageHandler()
{
	AuthInformationWindow = new QWidget();

	AuthInformationWindow->setFixedSize(480, 250);
	QGridLayout* gridLay = new QGridLayout();
	QLabel* lbChipIDHint = new QLabel("原始授权加密狗ID: ");
	QLineEdit* editChipID = new QLineEdit();
	editChipID->setReadOnly(true);
	editChipID->setText(QString::fromStdString(g_authInfo.chipID));
	QLabel* lbPCIDHint = new QLabel("原始授权PC: ");
	QLineEdit* editPCID = new QLineEdit();
	editPCID->setReadOnly(true);
	editPCID->setText(QString::fromStdString(g_authInfo.pcUUID));
	QLabel* lbVerifyCode = new QLabel("原始授权码: ");
	QLineEdit* editVerifyCode = new QLineEdit();
	editVerifyCode->setReadOnly(true);
	editVerifyCode->setText(QString::fromStdString(g_authInfo.authCode));
	QLabel* lbAuthTime = new QLabel("授权时间");
	QLineEdit* editAuthTime = new QLineEdit();
	editAuthTime->setReadOnly(true);
	editAuthTime->setText(QString::fromStdString(g_authInfo.authTime));

	gridLay->addWidget(lbChipIDHint, 0, 0, 1, 1);
	gridLay->addWidget(editChipID, 0, 1, 1, 3);
	gridLay->addWidget(lbPCIDHint, 1, 0, 1, 1);
	gridLay->addWidget(editPCID, 1, 1, 1, 3);
	gridLay->addWidget(lbVerifyCode, 2, 0, 1, 1);
	gridLay->addWidget(editVerifyCode, 2, 1, 1, 3);
	gridLay->addWidget(lbAuthTime, 3, 0, 1, 1);
	gridLay->addWidget(editAuthTime, 3, 1, 1, 3);
	AuthInformationWindow->setLayout(gridLay);
}
ScadaMessageHandler::~ScadaMessageHandler()
{

}

void ScadaMessageHandler::handleOpcDisconnected(OPClient* client)
{
	g_mainWindow->GetCanvasPanel()->hide();
	std::vector<std::function<void(void)>> callbacks;
	callbacks.push_back(nullptr);
	callbacks.push_back([&]() {
		ScadaScheduler* dispatcher = ScadaScheduler::GetInstance();
		SCT_SEQUENCE_TASK* reconnTask = new SCT_SEQUENCE_TASK();
		reconnTask->type = OPC_RECONNECT;
		dispatcher->AddTask(reconnTask);
	});
	callbacks.push_back(nullptr);
	HmiTemplateMsgBox::warning(nullptr, QString("错误"), QString("OPC UA服务器断开连接"), { "","重新连接","确定" }, callbacks);
}
void ScadaMessageHandler::handleOpcConnectSuccess(OPClient* client)
{
	HmiTemplateMsgBox::success(nullptr, QString("提示"), QString("OPC UA服务器连接成功"), { "","","确定" }, {nullptr,nullptr,nullptr});
}
void ScadaMessageHandler::handleOpcConnectFailed(OPClient* client)
{
	std::vector<std::function<void(void)>> callbacks;
	callbacks.push_back(nullptr);
	callbacks.push_back([&]() {
			g_opcuaClient->ReconnectWithHint();
		});
	callbacks.push_back(nullptr);
	HmiTemplateMsgBox::warning(nullptr, QString("警告"), QString("无法与OPC UA服务器创建连接"), { "","重新连接","确定" }, callbacks);
}
void ScadaMessageHandler::handleOpcServerReboot()
{

}
void ScadaMessageHandler::handleOpcReconnectFailed(OPClient* client)
{
	std::vector<std::function<void(void)>> callbacks;
	callbacks.push_back(nullptr);
	callbacks.push_back([&]() {
		ScadaScheduler* dispatcher = ScadaScheduler::GetInstance();
		SCT_SEQUENCE_TASK* reconnTask = new SCT_SEQUENCE_TASK();
		reconnTask->type = OPC_RECONNECT;
		dispatcher->AddTask(reconnTask);
	});
	callbacks.push_back(nullptr);
	HmiTemplateMsgBox::warning(nullptr, QString("错误"), QString("重连服务器失败"),{ "","重新连接","确定" }, callbacks);
}

void ScadaMessageHandler::handleOpcWriteFailed(OPClient* client, const QString& targetNode, const QString& targetValue, const QString& errorCode)
{
	HmiTemplateMsgBox::warning(nullptr, QString("错误"), QString("向节点%1写入值%2失败,错误码:%3")
		.arg(targetNode).arg(targetValue).arg(errorCode), {"","","确定"}, {nullptr,nullptr,nullptr});
}

void ScadaMessageHandler::handleDogPullOut()
{
	static bool triggered = false;
	if (!triggered)
	{
		triggered = true;
		int ret = QMessageBox::critical(NULL, QStringLiteral("加密狗校验错误"), "检测到加密狗已被拔出,请重新插入!", QMessageBox::Close);
		if (ret == QMessageBox::Close)
		{
			if (CheckAuth() != 0)
			{
				QApplication::exit(0);
			}
		}
		triggered = false;
	}
}

void ScadaMessageHandler::handleAuthNotMatch(MenuLayerTop* menu,const std::string& origin_chipId, const std::string& local_chipId, const std::string& origin_uuid, const std::string& local_uuid)
{
	QMessageBox hint;
	hint.setIcon(QMessageBox::Critical);
	hint.setWindowTitle("授权校验失败");

	QString message = QString("认证信息: \r\n当前加密狗ID: %1  \r\n 当前Host: %2\r\n 请联系管理员重新授权!").arg(QString::fromStdString(local_chipId)).arg(QString::fromStdString(local_uuid));
	hint.setText(message);
	hint.exec();
}

void ScadaMessageHandler::handleResoreHistory()
{
	std::vector<std::tuple<int, QString>> records = DataBaseCNC::GetInstance()->GetDraftOpenRecords();
	std::vector<std::function<void(void)>> callbacks;
	callbacks.push_back(nullptr);
	callbacks.push_back([&]() {
		std::vector<std::tuple<int, QString>> records = DataBaseCNC::GetInstance()->GetDraftOpenRecords();
		for (std::tuple<int, QString>& rec : records)
		{
			auto [stationId, filePath] = rec;
			std::shared_ptr sketch = g_mainWindow->sketchLists[stationId + 1];
			DXFProcessor processor(sketch);
			std::string file = filePath.toLocal8Bit().constData();
			processor.SetCompleteCallback([&]()
			{
				if(sketch->GetEntities().size())
				{
					auto groups = sketch->GetEntityGroups();
					for (EntGroup* group : groups)
					{
						for (EntRingConnection* ring : group->rings)
						{
							if (ring->direction == GeomDirection::CW)
							{
								ring->Reverse();
								ring->direction = GeomDirection::CCW;
							}
						}
					}
					sketch->SetOrigin(sketch->attachedOCS->objectRange->getMin());
					std::string NcProgram = sketch->ToNcProgram();
					g_mainWindow->infoPanel->updateStats(sketch.get());
					GCodeEditor::GetInstance()->setText(QString::fromStdString(NcProgram));
				}
			});
			bool success = processor.read(file);
		}
	});
	callbacks.push_back(nullptr);
	if (records.size())
	{
		HmiTemplateMsgBox::question(nullptr, QString("提示"), QString("检测到上次关闭的工程,是否恢复打开?"), { "","确定","取消" }, callbacks);
	}
}

void ScadaMessageHandler::showAuthWindow()
{
	AuthInformationWindow->show();
}
