#include "UI/SpinEditInput.h"
#include "UI/VirtualKeyBoard.h"
#include <iostream>
#include <QTimer>
#include <QFocusEvent>

namespace CNCSYS
{
	
	SpinEditInput::SpinEditInput(const QString& title,int minimumWidth, std::pair<int, int> range, const QString& suffix, int decimal, float startValue)
	{
		this->setStyleSheet(SpinBoxInputStyele);
		this->setMinimumWidth(100);
		this->setRange(range.first,range.second);
		this->setSuffix(suffix);
		this->setDecimals(decimal);
		this->setValue(startValue);
		this->title = title;
		this->startValue = startValue;
		this->installEventFilter(this);
	}
	bool SpinEditInput::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj == this)
		{
			switch (event->type())
			{
			case QEvent::FocusIn:
			{
				QFocusEvent* focusEvent = static_cast<QFocusEvent*>(event);
				if (focusEvent->reason() == QContextMenuEvent::Reason::Mouse)
				{
					lineEdit()->setCursorPosition(this->cleanText().length());
					VirtualKeyBoard* keyboard = VirtualKeyBoard::GetInstance();
					keyboard->disconnect();
					connect(keyboard, &VirtualKeyBoard::textChanged, [this](const QString& text) {this->setValue(text.toDouble()); });
					keyboard->SetHolder(this);
					keyboard->SetConfig(title, this->cleanText().toDouble());
					keyboard->move(this->mapToGlobal(QPoint(0, this->height())));
					keyboard->show();
					keyboard->raise(); // 置于同层级窗口最上方
				}
				break;
			}
			}
		}
		return QDoubleSpinBox::eventFilter(obj, event);
	}
}
