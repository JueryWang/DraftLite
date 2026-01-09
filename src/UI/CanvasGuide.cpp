#include "UI/CanvasGuide.h"
#include <QGridLayout>
#include <UI/Components/HmiInterfaceDefines.h>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

CanvasGuide* CanvasGuide::s_instance = nullptr;

CanvasGuide* CanvasGuide::GetInstance()
{
	if (CanvasGuide::s_instance == nullptr)
	{
		CanvasGuide::s_instance = new CanvasGuide();
	}
	return CanvasGuide::s_instance;
}

void CanvasGuide::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
	setGraphicsEffect(opacityEffect);

	QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity");
	animation->setDuration(1000);

	animation->setStartValue(0.0);
	animation->setEndValue(1.0);


	animation->setEasingCurve(QEasingCurve::Linear);

	animation->start(QPropertyAnimation::DeleteWhenStopped);

}

void CanvasGuide::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
	setGraphicsEffect(opacityEffect);

	QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity");
	animation->setDuration(1000);

	animation->setStartValue(1.0);
	animation->setEndValue(0.0);

	animation->setEasingCurve(QEasingCurve::Linear);

	animation->start(QPropertyAnimation::DeleteWhenStopped);

}

CanvasGuide::CanvasGuide(QWidget* parent)
{
	this->setWindowFlags(Qt::Tool);
	this->setAutoFillBackground(true);
	QGridLayout* layout = new QGridLayout(this);

	btnZoomIn = new QPushButton(this);
	QIcon iconZoomIn(ICOPATH(canvas_zoomin.png));
	btnZoomIn->setIcon(iconZoomIn);
	btnZoomIn->setIconSize(QSize(16, 16));
	layout->addWidget(btnZoomIn, 0, 0);
	longPressTimerZoomIn = new QTimer(this);
	longPressTimerZoomIn->setInterval(10);
	connect(btnZoomIn, &QPushButton::pressed, longPressTimerZoomIn, [&]()
		{
			longPressTimerZoomIn->start();
		});
	connect(btnZoomIn, &QPushButton::released, longPressTimerZoomIn, [&]()
		{
			longPressTimerZoomIn->stop();
		});

	btnViewUp = new QPushButton(this);
	QIcon iconViewUp(ICOPATH(canvas_moveup.png));
	btnViewUp->setIcon(iconViewUp);
	btnViewUp->setIconSize(QSize(16, 16));

	longPressTimerViewUp = new QTimer(this);
	longPressTimerViewUp->setInterval(20);
	connect(btnViewUp, &QPushButton::pressed, longPressTimerViewUp, [&]()
		{
			longPressTimerViewUp->start();
		});
	connect(btnViewUp, &QPushButton::released, longPressTimerViewUp, [&]()
		{
			longPressTimerViewUp->stop();
		});
	layout->addWidget(btnViewUp, 0, 1);

	btnZoomOut = new QPushButton(this);
	QIcon iconZoomOut(ICOPATH(canvas_zoomout.png));
	btnZoomOut->setIcon(iconZoomOut);
	btnZoomOut->setIconSize(QSize(16, 16));
	layout->addWidget(btnZoomOut, 0, 2);
	longPressTimerZoomOut = new QTimer(this);
	longPressTimerZoomOut->setInterval(10);
	connect(btnZoomOut, &QPushButton::pressed, longPressTimerZoomOut, [&]()
		{
			longPressTimerZoomOut->start();
		});
	connect(btnZoomOut, &QPushButton::released, longPressTimerZoomOut, [&]()
		{
			longPressTimerZoomOut->stop();
		});

	btnViewLeft = new QPushButton(this);
	QIcon iconViewLeft(ICOPATH(canvas_moveleft.png));
	btnViewLeft->setIcon(iconViewLeft);
	btnViewLeft->setIconSize(QSize(16, 16));
	layout->addWidget(btnViewLeft, 1, 0);
	longPressTimerViewLeft = new QTimer(this);
	longPressTimerViewLeft->setInterval(20);
	connect(btnViewLeft, &QPushButton::pressed, longPressTimerViewLeft, [&]()
		{
			longPressTimerViewLeft->start();
		});
	connect(btnViewLeft, &QPushButton::released, longPressTimerViewLeft, [&]()
		{
			longPressTimerViewLeft->stop();
		});

	btnViewDown = new QPushButton(this);
	QIcon iconViewDown(ICOPATH(canvas_movedown.png));
	btnViewDown->setIcon(iconViewDown);
	btnViewDown->setIconSize(QSize(16, 16));
	layout->addWidget(btnViewDown, 1, 1);
	longPressTimerViewDown = new QTimer(this);
	longPressTimerViewDown->setInterval(20);
	connect(btnViewDown, &QPushButton::pressed, longPressTimerViewDown, [&]()
		{
			longPressTimerViewDown->start();
		});
	connect(btnViewDown, &QPushButton::released, longPressTimerViewDown, [&]()
		{
			longPressTimerViewDown->stop();
		});

	btnViewRight = new QPushButton(this);
	QIcon iconViewRight(ICOPATH(canvas_moveright.png));
	btnViewRight->setIcon(iconViewRight);
	btnViewRight->setIconSize(QSize(16, 16));
	longPressTimerViewRight = new QTimer(this);
	longPressTimerViewRight->setInterval(20);
	connect(btnViewRight, &QPushButton::pressed, longPressTimerViewRight, [&]()
		{
			longPressTimerViewRight->start();
		});
	connect(btnViewRight, &QPushButton::released, longPressTimerViewRight, [&]()
		{
			longPressTimerViewRight->stop();
		});

	layout->addWidget(btnViewRight, 1, 2);
	layout->setSpacing(1);
	layout->setContentsMargins(5, 5, 5, 5);

	this->setLayout(layout);
	this->installEventFilter(this);
}

CanvasGuide::~CanvasGuide()
{

}
