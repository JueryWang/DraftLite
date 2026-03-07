#include "UI/MainLayer.h"
#include "UI/GLWidget.h"
#include "Graphics/Canvas.h"
#include "IO/DxfProcessor.h"
#include "IO/GCodeProcessor.h"
#include "IO/XMLProcessor.h"
#include "UI/TransformBaseHint.h"
#include "UI/VirtualKeyBoard.h"
#include "UI/ArrayGenerationDlg.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/ScadaMessageHandler.h"
#include "Graphics/Sketch.h"
#include "Graphics/Anchor.h"
#include "UI/SizeDefines.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/MotionControl.h"
#include "UI/DigitalHUD.h"
#include "UI/GCodeEditor.h"
#include "UI/SizeDefines.h"
#include "UI/MenuLayerTop.h"
#include "UI/Components/HmiTemplateWebViewer.h"
#include "UI/CanvasGuide.h"

#include <QApplication>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDir>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QSplitter>
#include "UI/ToolPanel.h"
#include <UI/MenuLayerTop.h>
#include <QTouchEvent>
#include <memory>

std::atomic<bool> g_timerThreadExit(false);


void TimerTask(MainLayer* layer)
{
	while (!g_timerThreadExit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
		layer->AutoSaveScene();
		//char KeyPath[MAX_PATH];
		//if (CheckAuth())
		//{
		//	QMetaObject::invokeMethod(ScadaMessageHandler::GetInstance(), "handleAuthCheckError", Qt::QueuedConnection);
		//}
	}
}

MainLayer::MainLayer(void* sketch, OverallWindow* ovWindow)
{
	g_mainWindow = this;
	loadQssTheme();
	//CreateMenus();

	setWindowFlags(Qt::FramelessWindowHint);
	HmiTemplateWebViewer::SetUrl("qrc:/qml/WebViewer.qml");
	webView = HmiTemplateWebViewer::GetWidget();
	webView->installEventFilter(this);
	webView->show();

	canvasOperationPanel = new QWidget(webView);
	this->setCentralWidget(webView);
	QVBoxLayout* ovlay = new QVBoxLayout(canvasOperationPanel);
	QHBoxLayout* hlayCol1 = new QHBoxLayout();

	//GPU版本
	if (CNCSYS::InitializeOpenGL())
	{
		CNCSYS::SketchGPU* gpuInstance = (CNCSYS::SketchGPU*)sketch;
		renderByGPU = true;
		mSketchGPU.reset(gpuInstance);
		int canvasWidth = ScreenSizeHintX(canvas_panel_width_ratio);
		int canvasHeight = ScreenSizeHintY(canvas_panel_height_ratio);
		fixed_canvas_aspect = canvasWidth / canvasHeight;
		CanvasGPU* canvasMain = new CanvasGPU(mSketchGPU, canvasWidth, canvasHeight, true);
		g_canvasInstance = canvasMain;
		glWidget = new GLWidget(canvasMain, nullptr, DYNAMIC_DRAW);
		canvasMain->SetFrontWidget(glWidget);
		canvasMain->drawTickers = true;
		canvasMain->drawAnchor = true;
		glWidget->setFixedSize(canvasWidth, canvasHeight);
		glWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		mGLWidget.reset(glWidget);

		m_bottomInfo = new QLabel();
		m_bottomInfo->setFixedHeight(40);
		m_bottomInfo->setStyleSheet("color: white;");
		canvasMain->hoverChangedCallback = [this](const QString& str) {m_bottomInfo->setText(str); };
		//vlay->addWidget(m_bottomInfo);

		//DigitalHUD* hud = new DigitalHUD(300,200);
		//hud->show();
		GCodeEditor::initFont(global_font_mp["Cascadia"], 10);
		GCodeEditor::initIntellisense();
		editor = GCodeEditor::GetInstance();
		editor->setFixedSize(ScreenSizeHintX(gcode_panel_width_ratio), ScreenSizeHintY(gcode_panel_height_ratio) + m_bottomInfo->height());
		editor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		editor->SetSketch(gpuInstance);
		MenuLayerTop* menuLayer = new MenuLayerTop(ovWindow);

		hlayCol1->addWidget(glWidget);
		hlayCol1->addWidget(editor);

		//MotionControl* mc = new MotionControl();
		//mc->setFixedSize(ScreenSizeHintX(1.0f), ScreenSizeHintY(MotionPanelHeight_Ratio));
		//hlayCol1->setSpacing(0);
		ovlay->addWidget(menuLayer);
		ovlay->addLayout(hlayCol1);
		//ovlay->setSpacing(0);
		//ovlay->addWidget(mc);
		ovlay->setContentsMargins(10, 0, 30, 10);
		canvasOperationPanel->setStyleSheet(R"(.QWidget { border: 2px solid #2c80d8; border-radius: 8px; padding: 10px; })");
		canvasOperationPanel->show();
		canvasOperationPanel->setLayout(ovlay);
		canvasOperationPanel->setWindowFlags(Qt::SubWindow);
		canvasOperationPanel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		canvasOperationPanel->move(20, move_height_ratio * screen_resolution_y);

		CanvasGuide* canvasGuide = CanvasGuide::GetInstance();
		canvasGuide->setParent(glWidget);
		canvasGuide->move(QPoint(50, 10));
		canvasGuide->hide();

		if (g_settings->value("Settings/Mode") != "Debug")
		{
			canvasOperationPanel->show();
		}
		//canvasOperationPanel->setMaximumSize(ScreenSizeHintX(1.0f), ScreenSizeHintY(1.0f));
		mOCSGPU = canvasMain->GetOCSSystem();
		this->show();

	}
	else
	{

	}
	this->showMaximized();

	//std::thread timerThread(TimerTask, this);
	//timerThread.detach();

}
MainLayer::~MainLayer()
{
	g_timerThreadExit = true;
	mSketchGPU.reset();
	ScadaScheduler::GetInstance()->Stop();
}

void MainLayer::loadQssTheme()
{
	m_qssPath = "./Resources/qss";
	QFile file(m_qssPath + "/default.qss");
	file.open(QIODevice::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	file.close();
	qApp->setStyleSheet(styleSheet);
}

QSize MainLayer::GetWindowSize()
{
	return this->size();
}

void MainLayer::changeEvent(QEvent* event)
{
	QWidget::changeEvent(event);
	if (event->type() == QEvent::WindowStateChange) {
		if (windowState() & Qt::WindowMinimized) {
			if (mGLWidget.get())
			{
				mGLWidget.get()->hide();
			}
		}
		else if (!(windowState() & (Qt::WindowMinimized | Qt::WindowMaximized))) {
			if (mGLWidget.get())
			{
				mGLWidget.get()->show();
			}
		}
	}
}

bool MainLayer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == webView)
	{
		//过滤Control事件
		if (event->type() == QEvent::Wheel) {
			QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
			if (wheelEvent->modifiers() & Qt::ControlModifier)
			{
				return true;
			}
		}

		else if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			if ((keyEvent->modifiers() & Qt::ControlModifier) &&
				(keyEvent->key() == Qt::Key_Plus ||
					keyEvent->key() == Qt::Key_Minus ||
					keyEvent->key() == Qt::Key_Equal)) {
				return true; // 拦截快捷键，阻止缩放
			}
		}
		//过滤触摸屏缩放事件
		else if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == event->type() == QEvent::TouchEnd)
		{
			event->accept();
			QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
			const QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

			if (touchPoints.size() >= 2)
			{
				return true;
			}
		}
	}
	return QWidget::eventFilter(obj, event);
}

void MainLayer::AutoSaveScene()
{
	QString AutoSavePath = QDir::currentPath() + "/temp/AtuoSave.xml";
}
void MainLayer::UpdateSize()
{
	//QScreen* screen = QGuiApplication::primaryScreen();
	//screen_resolution_x = screen->geometry().width();
	//screen_resolution_y = screen->geometry().height();
	//std::cout << "/n current screen size: " << screen_resolution_x << ", " << screen_resolution_y << std::endl;
	//glWidget->setFixedSize(ScreenSizeHintX(DrawPanelWidth_Ratio), ScreenSizeHintY(DrawPanelHeight_Ratio));
	//glWidget->GetContet()->window->Resize(QSize(ScreenSizeHintX(DrawPanelWidth_Ratio), ScreenSizeHintY(DrawPanelHeight_Ratio)));
	//editor->setFixedSize(ScreenSizeHintX(GCodePanelWidth_Ratio), ScreenSizeHintY(GCodePanelHeight_Ratio));
	//this->showMaximized();
}