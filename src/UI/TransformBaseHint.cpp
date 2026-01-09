#include "UI/TransformBaseHint.h"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QHBoxLayout>

static QString FocusInSpinStyle = QString(R"(
	QDoubleSpinBox {
		padding-right: 0px; /* 移除右侧预留的按钮空间 */
		background-color: #ffffff;
	}
	QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
		width: 0;
		height: 0;
		margin: 0;
		padding: 0;
	}
)");

static QString FoucusOutSpinStyle = QString(R"(
	QDoubleSpinBox {
		padding-right: 0px; /* 移除右侧预留的按钮空间 */
		background-color: #999999; /* 灰色背景 */
		color:#bab7af
	}
	QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
		width: 0;
		height: 0;
		margin: 0;
		padding: 0;
	}
)");

namespace CNCSYS
{
	MouseHoverEditSpin::MouseHoverEditSpin(TransformBaseHint* parent, EditSpinType type) : baseHint(parent),type(type)
	{
	}

	void MouseHoverEditSpin::keyPressEvent(QKeyEvent* event)
	{
		QDoubleSpinBox::keyPressEvent(event);
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		{
			if (type == EditSpinType::X)
			{
				baseHint->mouseSpinY->setFocus();
			}
			else
			{
				baseHint->mouseSpinX->setFocus();
			}
			this->clearFocus();
			emit ConfirmData(glm::vec3(baseHint->mouseSpinX->value(),baseHint->mouseSpinY->value(),0.0f));
		}
	}

	void MouseHoverEditSpin::focusInEvent(QFocusEvent* event)
	{
		QDoubleSpinBox::focusInEvent(event);
		setStyleSheet(FocusInSpinStyle);
	}

	void MouseHoverEditSpin::focusOutEvent(QFocusEvent* event)
	{
		QDoubleSpinBox::focusOutEvent(event);
		setStyleSheet(FoucusOutSpinStyle);
	}


	TransformBaseHint::TransformBaseHint(QWidget* parent)
	{
		this->setParent(parent);
		this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		setAttribute(Qt::WA_TranslucentBackground);  // 关键属性
		setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);
		QFont font = QFont("Microsoft YaHei", 12);
		QFontMetrics fmF(font);
		QRect box = fmF.boundingRect("请指定旋转起始点:");

		QHBoxLayout* hlay = new QHBoxLayout();
		hintLabel = new QLabel("请指定旋转基点:");
		hintLabel->setStyleSheet("background-color: rgba(57,77,106,195) !important; color: white;");
		hintLabel->setFont(font);
		hintLabel->setFixedWidth(box.width());
		hlay->addWidget(hintLabel);
		mouseSpinX = new MouseHoverEditSpin(this,EditSpinType::X);
		mouseSpinX->setFont(font);
		mouseSpinX->setStyleSheet(FocusInSpinStyle);
		mouseSpinX->setFixedWidth(65);
		mouseSpinX->setFocus();
		mouseSpinX->installEventFilter(this);
		mouseSpinX->setRange(-1000000, 10000000);
		mouseSpinY = new MouseHoverEditSpin(this, EditSpinType::Y);
		mouseSpinY->setFont(font);
		mouseSpinY->setFixedWidth(65);
		mouseSpinY->setStyleSheet(FoucusOutSpinStyle);
		mouseSpinY->installEventFilter(this);
		mouseSpinY->setRange(-1000000, 10000000);
		hlay->addWidget(mouseSpinX);
		hlay->addWidget(mouseSpinY);
		this->setLayout(hlay);
	}

	TransformBaseHint::~TransformBaseHint()
	{

	}
	void TransformBaseHint::ResetToRotate()
	{
		mouseSpinX->setFocus();
		mouseSpinX->selectAll();
		mouseSpinY->clearFocus();
		hintLabel->setText("请指定旋转基点:");
		mouseSpinY->setVisible(true);
	}
	void TransformBaseHint::ResetToScale()
	{
		mouseSpinX->setFocus();
		mouseSpinY->selectAll();
		mouseSpinY->cleanText();
		hintLabel->setText("请指定缩放基点");
		mouseSpinY->setVisible(true);
	}

	void TransformBaseHint::ResetToMirror()
	{
		mouseSpinX->setFocus();
		mouseSpinX->selectAll();
		mouseSpinY->clearFocus();
		hintLabel->setText("请指定镜面起始点:");
		mouseSpinY->setVisible(true);
	}
}
