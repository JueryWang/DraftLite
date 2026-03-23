#include "Common/Program.h"
#include "UI/Components/HmiTemplateMonitorTool.h"
#include "UI/Components/HmiTemplateMsgBox.h"
#include "NetWork/OPClient.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/ScadaMessageHandler.h"
#include "IO/ExcelProcessor.h"
#include <Controls/GlobalPLCVars.h>
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QDebug>

HmiTemplateConfigVarSettings::HmiTemplateConfigVarSettings(QSettings* config) : setting(config)
{
	QVBoxLayout* vlay = new QVBoxLayout();
	
	QGroupBox* protocolGroup = new QGroupBox("协议类型");
	QHBoxLayout* radioLayout = new QHBoxLayout();
	protoOPCUARadio = new QRadioButton("OPC UA");
	connect(protoOPCUARadio, &QRadioButton::toggled, this, [=](bool checked) {
		if (checked) {
			setting->setValue("Protocol","OPC UA");
		}
	});

	protoModbusTCPRadio = new QRadioButton("Modbus TCP");
	connect(protoOPCUARadio, &QRadioButton::toggled, this, [=](bool checked) {
		if (checked) {
			setting->setValue("Protocol", "Modbus TCP");
		}
	});
	
	protoModbusRTURadio = new QRadioButton("Modbus RTU");
	connect(protoModbusRTURadio, &QRadioButton::toggled, this, [=](bool checked)
	{
		if(checked){
			setting->setValue("Protocol","Modbus RTU");
		}
	});

	radioLayout->addWidget(protoOPCUARadio);
	radioLayout->addWidget(protoModbusTCPRadio);
	radioLayout->addWidget(protoModbusRTURadio);
	protocolGroup->setLayout(radioLayout);
	vlay->addWidget(protocolGroup);

	QGroupBox* varConfigGroup = new QGroupBox("变量设置");
	QHBoxLayout* configLayout = new QHBoxLayout();
	QLabel* labelUrl = new QLabel("IP地址");
	urlEdit = new QLineEdit("");
	QLabel* labelNodeAddress = new QLabel("节点地址");
	addressEdit = new QLineEdit("");
	configLayout->addWidget(labelUrl);
	configLayout->addWidget(urlEdit);
	configLayout->addWidget(labelNodeAddress);
	configLayout->addWidget(addressEdit);
	varConfigGroup->setLayout(configLayout);
	vlay->addWidget(varConfigGroup);

	QHBoxLayout* btnLayout = new QHBoxLayout();
	QSpacerItem* placeHolder = new QSpacerItem(120,25,QSizePolicy::Expanding);
	btnLayout->addSpacerItem(placeHolder);
	btnConfirm = new QPushButton("确定");
	connect(btnConfirm, &QPushButton::clicked, [this]() {
		if (protoOPCUARadio->isChecked())
		{
			setting->setValue("Protocol", "OPC UA");
		}else if (protoModbusTCPRadio->isChecked())
		{
			setting->setValue("Protocol","Modbus TCP");
		}else if (protoModbusRTURadio->isChecked())
		{
			setting->setValue("Protocol","Modbus RTU");
		}
		setting->setValue("Url", QVariant(urlEdit->text()));
		setting->setValue("NodeAddress", QVariant(addressEdit->text()));
		this->close();
	});

	btnCancel = new QPushButton("取消");
	connect(btnCancel, &QPushButton::clicked, [this]() {
		this->close();
	});
	btnLayout->addWidget(btnConfirm);
	btnLayout->addWidget(btnCancel);

	vlay->addLayout(btnLayout);
	this->setLayout(vlay);
}

HmiTemplateConfigVarSettings::~HmiTemplateConfigVarSettings()
{

}

HmiTemplateMonitorTool::HmiTemplateMonitorTool()
{
	CreateMenu();
	table = new HmiTemplateTableMonitor({});
	connect(table, &HmiTemplateTableMonitor::sizeChanged, this, &HmiTemplateMonitorTool::OnTableSizeChanged);
	this->setCentralWidget(table);
	setting = new QSettings("Url","opc.tcp://192.168.6.6:4840");
	configPanel = new HmiTemplateConfigVarSettings(setting);
	this->setMinimumSize(1000,600);
}

HmiTemplateMonitorTool::~HmiTemplateMonitorTool()
{
	delete configPanel;
}

void HmiTemplateMonitorTool::OnConnectOPCUA()
{
	if (g_opcuaClient == nullptr)
	{
		g_opcuaClient = new OPClient();
	}
	QString url = setting->value("Url").toString();
	if (url.isEmpty())
		url = g_plcUrl;

	if (!url.isEmpty())
	{
		QByteArray byte = url.toLocal8Bit();
		if (g_opcuaClient->ConnectToServer(byte.data()))
		{
			QMetaObject::invokeMethod(ScadaMessageHandler::GetInstance(), "handleOpcConnectSuccess",
				Qt::QueuedConnection,
				Q_ARG(OPClient*, g_opcuaClient));
			ScadaScheduler::GetInstance()->SetOPCommClient(g_opcuaClient);

			QString nodeConfig = setting->value("NodeAddress").toString();
			if (nodeConfig.isEmpty())
				nodeConfig = g_plcSearchRootNode;

			if (!nodeConfig.isEmpty())
			{
				ScadaScheduler::GetInstance()->Stop();
				int colconIndex = nodeConfig.indexOf(':');
				QString ns = nodeConfig.left(colconIndex).trimmed();
				QString identifier = nodeConfig.mid(colconIndex + 1).trimmed();
				g_opcuaClient->InitDirTable(ns.toInt(), identifier.toStdString());
				table->CreateTableFromPLCVarMap();
			}
			ScadaScheduler::GetInstance()->Start();
		}
		else
		{
			HmiTemplateMsgBox::warning(this, "错误", "无法连接OPC UA服务器,请检查网络地址", { "","","确定" }, { nullptr,nullptr,nullptr });
		}
	}

}

void HmiTemplateMonitorTool::OnExportExcel()
{
	ExcelProcessor writer;
	writer.WritePLC_OPCUAVariantMap();
}

void HmiTemplateMonitorTool::OnImportExcel()
{
	ExcelProcessor reader;
	if(!g_opcuaClient)
		g_opcuaClient = new OPClient();

	auto now = std::chrono::system_clock::now();

	reader.ReadPLCVariantMap();
	QApplication::setOverrideCursor(Qt::WaitCursor);
	//table->CreateTableFromPLCVarMap();

	QString url = setting->value("Url").toString();
	if (url.isEmpty())
		url = g_plcUrl;
	if (!url.isEmpty())
	{
		QByteArray byte = url.toLocal8Bit();
		if (g_opcuaClient->ConnectToServer(byte.data()))
		{
			QMetaObject::invokeMethod(ScadaMessageHandler::GetInstance(), "handleOpcConnectSuccess",
				Qt::QueuedConnection,
				Q_ARG(OPClient*, g_opcuaClient));
			ScadaScheduler::GetInstance()->SetOPCommClient(g_opcuaClient);
			ScadaScheduler::GetInstance()->Start();
		}
		else
		{
			HmiTemplateMsgBox::warning(this, "错误", "无法连接OPC UA服务器,请检查网络地址", { "","","确定" }, { nullptr,nullptr,nullptr });
		}
	}

	auto end = std::chrono::system_clock::now();
	std::cout << "read from excel cost " << std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count() << " ms" << std::endl;
	QApplication::restoreOverrideCursor();
}

void HmiTemplateMonitorTool::OnTableSizeChanged(const QSize& size)
{
	this->resize(size);
}

void HmiTemplateMonitorTool::OnConfigVariant()
{
	configPanel->show();
}

void HmiTemplateMonitorTool::CreateMenu()
{
	QMenuBar* menuBar = this->menuBar();
	menuBar->setObjectName("hmiTemplate");
	QMenu* configMenu = menuBar->addMenu("配置");

	QAction* actConfigVar = configMenu->addAction("变量表");
	connect(actConfigVar, &QAction::triggered, this,&HmiTemplateMonitorTool::OnConfigVariant);
	QAction* actConnectServer = configMenu->addAction("连接服务器");
	connect(actConnectServer, &QAction::triggered, this, &HmiTemplateMonitorTool::OnConnectOPCUA);
	QAction* actReadFromExcel = configMenu->addAction("从Excel表格导入");
	connect(actReadFromExcel, &QAction::triggered, this, &HmiTemplateMonitorTool::OnImportExcel);
	QAction* actionExportToExcel = configMenu->addAction("导出变量表到Excel");
	connect(actionExportToExcel, &QAction::triggered, this, &HmiTemplateMonitorTool::OnExportExcel);
}