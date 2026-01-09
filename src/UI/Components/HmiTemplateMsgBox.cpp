#include "UI/Components/HmiTemplateMsgBox.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>

HmiTemplateMsgBox::HmiTemplateMsgBox(QWidget* parent, const QString& mainMsg, const QString& attachedMsg)
{
	handler = new QWidget(this);
	handler->setFixedSize(QGuiApplication::primaryScreen()->geometry().size().width() * msgbox_width_refactor,
		QGuiApplication::primaryScreen()->geometry().size().height() * msgbox_height_refactor);
	handler->setProperty("class", "blackWidget");

	this->setAttribute(Qt::WA_TranslucentBackground, true);
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);
	this->setWindowModality(Qt::WindowModal);
	this->setVisible(true);
	handler->installEventFilter(this);

	QVBoxLayout* vlay = new QVBoxLayout();
	QHBoxLayout* hlay = new QHBoxLayout();
	QHBoxLayout* btnsLay = new QHBoxLayout();
	icon = new QLabel();
	icon->setFixedSize(65, 65);

	hlay->addWidget(icon);
	hlay->addSpacing(20);
	QFont textFont("Microsoft YaHei", 10);
	QVBoxLayout* discripLayout = new QVBoxLayout();
	maindisp = new QLabel();
	maindisp->setObjectName("MsgBoxMainDisp");

	maindisp->setFont(textFont);
	maindisp->setFixedHeight(30);
	maindisp->setWordWrap(true);
	maindisp->setText(mainMsg);
	discripLayout->addWidget(maindisp);
	attachedDisp = new QLabel();
	attachedDisp->setObjectName("MsgBoxAttachedDisp");

	attachedDisp->setFont(textFont);
	attachedDisp->setWordWrap(true);
	attachedDisp->setText(attachedMsg);
	discripLayout->addWidget(attachedDisp);
	discripLayout->addSpacing(50);
	hlay->addLayout(discripLayout);
	vlay->addLayout(hlay);

	btn1 = new QPushButton();
	connect(btn1, &QPushButton::clicked, this, &HmiTemplateMsgBox::on_clcBtn1);
	btn2 = new QPushButton();
	connect(btn2, &QPushButton::clicked, this, &HmiTemplateMsgBox::on_clcBtn2);
	btn3 = new QPushButton();
	connect(btn3, &QPushButton::clicked, this, &HmiTemplateMsgBox::on_clcBtn3);

	QFont btnFont("Arial Black", 10, 75);
	btn1->setObjectName("MsgBoxBtn1");
	btn1->setFont(btnFont);
	btn2->setObjectName("MsgBoxBtn2");
	btn2->setFont(btnFont);
	btn3->setObjectName("MsgBoxBtn3");
	btn3->setFont(btnFont);
	btnsLay->setSpacing(10);
	btnsLay->addSpacing(80);

	btnsLay->addWidget(btn1);
	btnsLay->addWidget(btn2);
	btnsLay->addWidget(btn3);

	vlay->addLayout(btnsLay);
	handler->setLayout(vlay);
}
HmiTemplateMsgBox::~HmiTemplateMsgBox()
{

}
HmiTemplateMsgBox* HmiTemplateMsgBox::question(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks)
{
	static HmiTemplateMsgBox* questMsg = new HmiTemplateMsgBox(parent, title, text);
	questMsg->setWindowTitle(title);
	questMsg->setContent(text);
	questMsg->setButtonTexts(btnTexts);
	questMsg->setCallbacks(callbacks);
	QPixmap pix(ICOPATH(quest.svg));
	questMsg->icon->setPixmap(pix);
	questMsg->move((screen_resolution_x - questMsg->width()) / 2, (screen_resolution_y - questMsg->height()) / 2);
	questMsg->show();
	questMsg->handler->activateWindow();
	questMsg->handler->setFocus();

	return questMsg;
}
HmiTemplateMsgBox* HmiTemplateMsgBox::warning(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks)
{
	static HmiTemplateMsgBox* warningMsg = new HmiTemplateMsgBox(parent, title, text);
	warningMsg->setWindowTitle(title);
	warningMsg->setContent(text);
	warningMsg->setButtonTexts(btnTexts);
	warningMsg->setCallbacks(callbacks);
	QPixmap pix(ICOPATH(warn.svg));
	warningMsg->icon->setPixmap(pix);
	warningMsg->move((screen_resolution_x - warningMsg->width()) / 2, (screen_resolution_y - warningMsg->height()) / 2);
	warningMsg->show();
	warningMsg->handler->activateWindow();
	warningMsg->handler->setFocus();

	return warningMsg;
}

HmiTemplateMsgBox* HmiTemplateMsgBox::success(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks)
{
	static HmiTemplateMsgBox* successMsg = new HmiTemplateMsgBox(parent, title, text);
	successMsg->setWindowTitle(title);
	successMsg->setContent(text);
	successMsg->setButtonTexts(btnTexts);
	successMsg->setCallbacks(callbacks);
	QPixmap pix(ICOPATH(success.png));
	successMsg->icon->setPixmap(pix.scaled(
		65, 65,
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	));
	successMsg->move((screen_resolution_x - successMsg->width()) / 2, (screen_resolution_y - successMsg->height()) / 2);
	successMsg->show();
	successMsg->handler->activateWindow();
	successMsg->handler->setFocus();

	return successMsg;
}

void HmiTemplateMsgBox::setTitle(const QString& title)
{
	maindisp->setText(title);
}

void HmiTemplateMsgBox::setContent(const QString& content)
{
	attachedDisp->setText(content);
}

void HmiTemplateMsgBox::setButtonTexts(const QStringList& texts)
{
	if (texts[0] != "") { btn1->setText(texts[0]); btn1->show(); }
	else btn1->hide();

	if (texts[1] != "") { btn2->setText(texts[1]); btn2->show(); }
	else btn2->hide();

	if (texts[2] != "") { btn3->setText(texts[2]); btn3->show(); }
	else btn3->hide();
}

void HmiTemplateMsgBox::setCallbacks(const std::vector<std::function<void(void)>>& callbacks)
{
	if (callbacks[0] != nullptr) cb1 = callbacks[0];
	if (callbacks[1] != nullptr) cb2 = callbacks[1];
	if (callbacks[2] != nullptr) cb3 = callbacks[2];
}

void HmiTemplateMsgBox::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	// 每次显示时重新计算居中位置
	QScreen* screen = QApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int x = (screenRect.width() - width()) / 2;
	int y = (screenRect.height() - height()) / 2;
	move(x, y);
}

void HmiTemplateMsgBox::on_clcBtn1()
{
	this->hide();

	if (cb1)
		cb1();

}

void HmiTemplateMsgBox::on_clcBtn2()
{
	this->hide();

	if (cb2)
		cb2();

}

void HmiTemplateMsgBox::on_clcBtn3()
{
	this->hide();

	if (cb3)
		cb3();

}

bool HmiTemplateMsgBox::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type()) {
	case QEvent::MouseButtonPress:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton)
		{
			isDraging = true;
			offsetPoint = e->globalPosition().toPoint() - this->frameGeometry().topLeft();
		}
		event->accept();
		return true;
	}
	case QEvent::MouseMove:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (isDraging)
		{
			this->move(e->globalPosition().toPoint() - offsetPoint);
		}
		event->accept();
		return true;
	}
	case QEvent::MouseButtonRelease:
	{
		auto e = dynamic_cast<QMouseEvent*>(event);
		if (e->button() == Qt::LeftButton)
		{
			isDraging = false;
		}
		return true;
	}
	case QEvent::FocusOut:
	{
		//this->hide();
	}
	}
	return QWidget::eventFilter(obj, event);
}
