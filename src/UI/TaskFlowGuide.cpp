#include "UI/TaskFlowGuide.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/OverallWindow.h"
#include "UI/TaskListWindow.h"

#include <QEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QVBoxLayout>
#include <QLabel>

TaskFlowGuide::TaskFlowGuide(QWidget* parent) : QWidget(parent)
{
	this->setParent(parent);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setStyleSheet(R"(
		QWidget {
			background-color: rgba(255, 255, 255, 200);
			border-radius: 8px;
		}
	)");
	this->setFixedSize(45, 85);
	this->setGeometry(0, (screen_resolution_y - this->height()) / 2, this->width(), this->height());

	QVBoxLayout* layout = new QVBoxLayout();
	QLabel* labelFolder = new QLabel();
	labelFolder->setPixmap(QPixmap(ICOPATH(folder.png)).scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	layout->addWidget(labelFolder, 0, Qt::AlignCenter);
	QLabel* labelHint = new QLabel("任务");
	layout->addWidget(labelHint, 0, Qt::AlignCenter);
	this->setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	this->installEventFilter(this);
}

TaskFlowGuide::~TaskFlowGuide()
{

}

bool TaskFlowGuide::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Enter:
	{
		this->setCursor(Qt::SizeAllCursor);
		break;
	}
	case QEvent::Leave:
	{
		this->setCursor(Qt::ArrowCursor);
		break;
	}
	case QEvent::MouseButtonPress:
	{
		auto e = static_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton) {
			isDragging = true;
			lastGlobalMousePos = e->globalPos();
		}
		event->accept();
		return true;
	}
	case QEvent::MouseMove:
	{
		auto e = static_cast<QMouseEvent*>(event);
		if (isDragging) {
			QPoint curGlobalMousePos = e->globalPos();
			QPoint offset = curGlobalMousePos - lastGlobalMousePos;
			QPoint newWidgetPos = this->pos() + offset;
			newWidgetPos.setX(0);
			this->move(newWidgetPos);
			lastGlobalMousePos = curGlobalMousePos;
		}
		event->accept();
		return true;
	}
	case QEvent::MouseButtonRelease:
	{
		auto e = static_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton) {
			isDragging = false;
		}
		event->accept();
		return true;
	}
	case QEvent::MouseButtonDblClick:
	{
		TaskListWindow* taskList = TaskListWindow::GetInstance();
		taskList->setVisible(!taskList->isVisible());
		taskList->raise();
		taskList->setGeometry(0, screen_resolution_y - taskList->height() - 30, taskList->width(), taskList->height());
		break;
	}
	}
	return QWidget::eventFilter(obj, event);
}
