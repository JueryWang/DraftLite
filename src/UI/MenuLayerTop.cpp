#include "UI/MenuLayerTop.h"
#include "UI/SizeDefines.h"
#include "UI/MainLayer.h"
#include "UI/OverallWindow.h"
#include "UI/GCodeEditor.h"
#include "UI/Configer/WorkBlankConfig.h"
#include "UI/Configer/RoughingConfig.h"
#include "IO/DxfProcessor.h"
#include "IO/GCodeProcessor.h"
#include "IO/XMLProcessor.h"
#include "IO/Utils.h"
#include "Graphics/Sketch.h"
#include "Graphics/Point2D.h"
#include "Auth/WG_Authorization.h"
#include <QDir>
#include <QGridLayout>
#include <QCoreApplication>
#include <QToolButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>

using namespace CNCSYS;

MenuLayerTop::MenuLayerTop(OverallWindow* parent)
{
	setParent(parent);
	this->ovWindow = parent;
	topMenus = new QMenuBar(this);
	topMenus->setFixedHeight(30);
	topMenus->setObjectName("blackMenu");
	layout = new QHBoxLayout(this);

	fileMenu = topMenus->addMenu(tr("文件"));
	QAction* actImportDxf = fileMenu->addAction(tr("导入dxf"));
	connect(actImportDxf, &QAction::triggered, this, &MenuLayerTop::OnImportDxf);
	QAction* actImportDwg = fileMenu->addAction(tr("导入dwg"));
	connect(actImportDwg, &QAction::triggered, this, &MenuLayerTop::OnImportDwg);
	QAction* actExportNC = fileMenu->addAction(tr("导出为NC"));
	connect(actExportNC, &QAction::triggered, this, &MenuLayerTop::OnExportNC);
	QAction* actExportScene = fileMenu->addAction(tr("导出工程"));
	connect(actExportScene, &QAction::triggered, this, &MenuLayerTop::OnExportScene);
	QAction* actImportScene = fileMenu->addAction("导入工程");
	connect(actImportScene, &QAction::triggered, this, &MenuLayerTop::OnImportScene);

	QMenu* editMenu = topMenus->addMenu(tr("编辑"));
	QMenu* captureMenu = editMenu->addMenu(tr("开启图元捕捉"));
	actEntityCapture = captureMenu->addAction(tr("开启图元捕捉")); 
	actEntityCapture->setCheckable(true);
	actEntityCapture->setChecked(true);
	actPointCapture = captureMenu->addAction(tr("开启点捕捉"));
	actPointCapture->setCheckable(true);
	connect(actEntityCapture, &QAction::triggered, [&]() {
		actPointCapture->setChecked(false);
		ovWindow->mainWindow->mSketchGPU.get()->GetCanvas()->SetCaptureMode(CaptureMode::Entity);
		});
	connect(actPointCapture, &QAction::triggered, [&]() {
		actEntityCapture->setChecked(false);
		ovWindow->mainWindow->mSketchGPU.get()->GetCanvas()->SetCaptureMode(CaptureMode::Point);
		});
	QMenu* showModeMenu = editMenu->addMenu(tr("显示"));
	actShowArrow = showModeMenu->addAction(tr("方向箭头"));
	actShowArrow->setCheckable(true);
	connect(actShowArrow, &QAction::triggered, [&]() {
		bool checked = actShowArrow->isChecked();
		ovWindow->mainWindow->mSketchGPU.get()->GetCanvas()->showArrow = checked;
		});

	QAction* measureAct = editMenu->addAction(tr("测量"));
	connect(measureAct, &QAction::triggered, [&]() {
			ovWindow->mainWindow->mSketchGPU.get()->GetCanvas()->EnterModal(ModalState::MeasureDimension);
		});

	QMenu* addMenu = topMenus->addMenu(tr("添加"));
	QAction* addWorkBlance = addMenu->addAction(tr("毛坯"));

	QMenu* CamMenu = topMenus->addMenu(tr("CAM"));
	QAction* addRoughting = CamMenu->addAction(tr("开粗"));
	connect(addRoughting, &QAction::triggered, [&]() {
		RoughingConfigPage::GetInstance()->show();
	});

	connect(addWorkBlance, &QAction::triggered, [&]() {
		if (g_canvasInstance->GetSelectedEntitys().size())
		{
			CNCSYS::EntRingConnection* ring = g_canvasInstance->GetSelectedEntitys()[0]->ringParent;
			WorkBlankConfigPage::BindRing(ring);
			
		}
		WorkBlankConfigPage::GetInstance()->show();
	});

	QMenu* AuthMenu = topMenus->addMenu(tr("用户"));
	actAuthInformation = AuthMenu->addAction(tr("授权信息"));

	topMenus->setFixedWidth(ScreenSizeHintX(canvas_panel_width_ratio));
	topMenus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	//QToolButton* closeBtn = new QToolButton(this);
	//closeBtn->setFixedSize(30, 30);
	//closeBtn->setFixedSize(30, 30);
	//closeBtn->setStyleSheet(".QToolButton{background-color:transparent;border:1px solid rgba(255,255,255,0);\
		//							qproperty-icon: url(Resources/icon/close.png);qproperty-iconSize: 20px 20px;}\
		//			 .QToolButton:hover,pressed,selected{padding:0px 0px;background-color:rgba(226, 46, 39, 1.0)}");
		//closeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		//connect(closeBtn, &QToolButton::clicked, []()
		//	{
		//		PLC_TYPE_BOOL busy;
		//		ReadPLC_OPCUA(g_ConfigableKeys["AutoBusy"].c_str(), &busy, AtomicVarType::BOOL);
		//		if (busy)
		//		{
		//			QMessageBox::warning(nullptr, "警告", "设备正在自动运行中,请先关闭后退出!");
		//			return;
		//		}
		//		QCoreApplication::exit();
		//	});

		//QToolButton* minimizeBtn = new QToolButton(this);
		//minimizeBtn->setFixedSize(30, 30);
		//minimizeBtn->setStyleSheet(".QToolButton{background-color:transparent;border:1px solid rgba(255,255,255,0);\
		//						qproperty-icon: url(Resources/icon/minimize.png);qproperty-iconSize: 20px 20px;}\
		//			 .QToolButton:hover,pressed,selected{padding:0px 0px;background-color:rgba(61, 68, 80, 1.0)}");
		//minimizeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		//connect(minimizeBtn, &QToolButton::clicked, [&]()
		//	{
		//		ovWindow->showMinimized();
		//	});

	layout->addWidget(topMenus, 0, Qt::AlignLeft | Qt::AlignTop);
	//layout->addStretch();
	//layout->addWidget(minimizeBtn, 0, Qt::AlignTop | Qt::AlignRight);
	//layout->addWidget(closeBtn, 0, Qt::AlignTop | Qt::AlignRight);
	layout->setContentsMargins(0, 0, 0, 0);
	//this->installEventFilter(this);
	this->setLayout(layout);

	AuthInformation = new QGroupBox("授权信息");
	AuthInformation->setFixedSize(480, 250);
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
	AuthInformation->setLayout(gridLay);

	connect(actAuthInformation, &QAction::triggered, [this]()
		{
			AuthInformation->show();
		});
}

MenuLayerTop::~MenuLayerTop()
{

}

bool MenuLayerTop::eventFilter(QObject* obj, QEvent* event) {

	switch (event->type()) {
	case QEvent::MouseButtonPress:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton)
		{
			m_isDraging = true;
			m_offsetPoint = e->globalPosition().toPoint() - ovWindow->frameGeometry().topLeft();
		}
		event->accept();
		return true;
	}
	case QEvent::MouseMove:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (m_isDraging)
		{
			ovWindow->move(e->globalPosition().toPoint() - m_offsetPoint);
		}
		event->accept();
		return true;
	}
	case QEvent::MouseButtonRelease:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton)
		{
			m_isDraging = false;
		}
		return true;
	}
	case QEvent::MouseButtonDblClick:
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton)
		{
			ovWindow->mainWindow->UpdateSize();
		}
	}
	}

	return QWidget::eventFilter(obj, event);
}

void MenuLayerTop::ImportDxf(const QString& dxfFile)
{
	if (!dxfFile.isEmpty())
	{
		DXFProcessor processor(this->ovWindow->mainWindow->mSketchGPU);
		GCodeEditor::GetInstance()->clear();
		std::string file = dxfFile.toLocal8Bit().constData();
		processor.SetCompleteCallback([&]()
			{
				if (this->ovWindow->mainWindow->mSketchGPU.get()->GetEntities().size())
				{
					auto groups = this->ovWindow->mainWindow->mSketchGPU.get()->GetEntityGroups();
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
					this->ovWindow->mainWindow->mSketchGPU.get()->SetOrigin(this->ovWindow->mainWindow->mSketchGPU->attachedOCS->objectRange->getMin());
					this->setWindowTitle(dxfFile);
					std::string NcProgram = this->ovWindow->mainWindow->mSketchGPU.get()->ToNcProgram();
					GCodeEditor::GetInstance()->setText(QString::fromStdString(NcProgram));
					//Anchor::GetInstance()->ReAssignDataSize(GCodeEditor::GetInstance()->lines());
				}
			});
		processor.read(file);
	}
}


void MenuLayerTop::OnImportDxf()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		("打开文件"),
		QDir::currentPath(),
		("DXF文件(*.dxf);;所有文件(*)")
	);

	ImportDxf(fileName);
}
void MenuLayerTop::OnImportDwg()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		("打开文件"),
		QDir::currentPath(),
		("DWG文件(*.dwg);;所有文件")
	);

	QDir tempDir("temp");
	if (!tempDir.exists())
	{
		tempDir.mkpath(".");
	}
	if (!fileName.isEmpty())
	{
		QString dwgFileName = SplitFileFromPath(fileName);
		copyFileToDir(fileName,"./temp");

		QProcess dwg2dxf;
		dwg2dxf.setWorkingDirectory(QDir::currentPath() + "/temp");
		dwg2dxf.setProgram(QDir::currentPath()+"/dwg2dxf.exe");
		dwg2dxf.setArguments({dwgFileName});
		dwg2dxf.start();
		if (!dwg2dxf.waitForStarted()) {
			QMessageBox::information(nullptr,tr("错误"),tr("dwg格式有错误"));
			return;
		}
		else
		{
			int exitCode = dwg2dxf.exitCode();
			QByteArray stderrData = dwg2dxf.readAllStandardError();
			QString command = QDir::currentPath() + "/dwg2dxf.exe" + " ./temp/" + dwgFileName;
			system(command.toLocal8Bit());
			if (exitCode)
			{
				QMessageBox::warning(nullptr, QString("警告"), QString("dwg文件解析出错:%1").arg(QString::fromLocal8Bit(stderrData)));
			}
		}

	}

	QString dxfFile = FileNameFromPath(fileName) + ".dxf";
	ImportDxf(QDir::currentPath() + "/temp/" + dxfFile);
	this->ovWindow->mainWindow->mSketchGPU.get()->source = fileName.toStdString();
}
void MenuLayerTop::OnExportNC()
{
	QString ncFile = QFileDialog::getSaveFileName(nullptr,
		"选择保存的NC文件",
		QDir::homePath(),
		"CNC文件(*.nc);;所有文件(*)"
	);

	GCodeProcessor processor(this->ovWindow->mainWindow->mSketchGPU.get());
	processor.SaveToFile(ncFile.toLocal8Bit().data());
}
void MenuLayerTop::OnExportScene()
{
	QString fileName = QFileDialog::getSaveFileName(
		this,
		("导出工程"),
		QDir::homePath(),
		"wproj文件(*.wproj);;所有文件"
	);

	XMLProcessor processor;
	processor.SaveProject(fileName);
}
void MenuLayerTop::OnImportScene()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		("导入工程"),
		QDir::homePath(),
		"wproj文件(*.wproj);;所有文件"
	);

	XMLProcessor processor;
	if (!fileName.isEmpty())
	{
		processor.ReadProject(fileName);
	}
}