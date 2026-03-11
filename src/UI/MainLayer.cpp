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
#include "UI/Components/HmiTemplateStationPreview.h"
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
		std::shared_ptr<SketchGPU> sketch1(new SketchGPU());
		renderByGPU = true;
		DXFProcessor p(sketch1);
		std::wstring widePath = L"C:/WJH/Test/工具/测试Dxf/Labubu Keychains-DXFDOWNLOADS.COM.dxf";

		// 转换为 std::string（UTF-8 编码）
		std::string utf8Path = std::filesystem::path(widePath).string();
		p.read(utf8Path);
		//std::vector<glm::vec3> controlPoints;
		//for (int i = 0; i < 10; i++)
		//{
		//	controlPoints.push_back(glm::vec3(generateRandomNumber0To300(), generateRandomNumber0To300(), 0));
		//}
		//std::vector<float> knots = MathUtils::GenerateClampedKnots(controlPoints.size(), 3);
		//spline1->SetParameter(controlPoints,knots,false);
		//sketch1->AddEntity(spline1);
		int canvasWidth = ScreenSizeHintX(canvas_panel_width_ratio);
		int canvasHeight = ScreenSizeHintY(canvas_panel_height_ratio);
		fixed_canvas_aspect = (float)canvasWidth / canvasHeight;
		CanvasGPU* canvas1 = new CanvasGPU(sketch1, 400 * fixed_canvas_aspect, 400, false);


		GLWidget* preview1 = new GLWidget(canvas1, sketch1.get(), DYNAMIC_DRAW);
		preview1->setFixedSize(400 * fixed_canvas_aspect, 400);
		canvas1->SetFrontWidget(preview1);

		PreviewItem* previewItem1 = new PreviewItem(preview1);

		ovlay->setContentsMargins(10, 0, 30, 10);
		ovlay->addWidget(previewItem1);
		canvasOperationPanel->show();
		canvasOperationPanel->setLayout(ovlay);
		canvasOperationPanel->setWindowFlags(Qt::SubWindow);
		canvasOperationPanel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		canvasOperationPanel->move(20, move_height_ratio * screen_resolution_y);

		CanvasGuide* canvasGuide = CanvasGuide::GetInstance();
		canvasGuide->setParent(preview1);
		canvasGuide->move(QPoint(50, 10));
		canvasGuide->hide();

		if (g_settings->value("Settings/Mode") != "Debug")
		{
			canvasOperationPanel->show();
		}
		//canvasOperationPanel->setMaximumSize(ScreenSizeHintX(1.0f), ScreenSizeHintY(1.0f));
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