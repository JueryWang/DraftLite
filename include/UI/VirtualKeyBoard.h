#pragma once
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>
#include "UI/ClickableLabel.h"

namespace CNCSYS
{
	class SpinEditInput;

	class VirtualKeyBoard : public QWidget 
	{
		Q_OBJECT
	public:
		static VirtualKeyBoard* GetInstance();
		void SetConfig(const QString& title, double initialValue);
		void SetValue(double value);
		void SetHolder(SpinEditInput* spinBox);

	private:
		VirtualKeyBoard(QWidget* parent = nullptr,const QString& label = "", double initialValue = 0.0);
		~VirtualKeyBoard();

	signals:
		void ConfirmData(double value);
		void textChanged(const QString& text);

	private:
		static VirtualKeyBoard* instance;
		QLabel* title = nullptr;
		QLabel* numberBox = nullptr;
		ClickableLabel* closeBtn = nullptr;
		QPushButton* input0 = nullptr;
		QPushButton* input1 = nullptr;
		QPushButton* input2 = nullptr;
		QPushButton* input3 = nullptr;
		QPushButton* input4 = nullptr;
		QPushButton* input5 = nullptr;
		QPushButton* input6 = nullptr;
		QPushButton* input7 = nullptr;
		QPushButton* input8 = nullptr;
		QPushButton* input9 = nullptr;
		QPushButton* inputDot = nullptr;
		QPushButton* inputSign = nullptr;
		QPushButton* inputClear = nullptr;
		QPushButton* inputOK = nullptr;

		QString valueStr = "";

		SpinEditInput* holder = nullptr;
	};
}
