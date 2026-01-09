#pragma once

#include <QString>
#include <QDoubleSpinBox>

namespace CNCSYS
{
	static QString SpinBoxInputStyele = R"(
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
	QDoubleSpinBox:read-only{
		background-color: #f0f0f0;
		color: rgba(0,0,0,150);
		border: 1px solid #ccc;
		border-radius:3px;
	}
	)";

	class SpinEditInput : public QDoubleSpinBox
	{
		Q_OBJECT
	public:
		SpinEditInput(const QString& title,int minimumWidth, std::pair<int, int> range, const QString& suffix, int decimal, float startValue);
		~SpinEditInput() = default;

	protected:
		virtual bool eventFilter(QObject* obj, QEvent* event) override;

		QString title;
		float startValue = 0.0f;
	public:
		bool unFocusByClose = false;
	};
}
