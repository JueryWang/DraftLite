#include "UI/VirtualKeyBoard.h"
#include "UI/SpinEditInput.h"
#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace CNCSYS
{
	VirtualKeyBoard* VirtualKeyBoard::instance = nullptr;

	VirtualKeyBoard* VirtualKeyBoard::GetInstance()
	{
		if (instance == nullptr)
		{
			QMainWindow* mainWindow = qobject_cast<QMainWindow*>(QApplication::activeWindow());
			instance = new VirtualKeyBoard(mainWindow,"", 0);
		}
		return instance;
	}
	VirtualKeyBoard::VirtualKeyBoard(QWidget* parent, const QString& label, double initialValue) : QWidget(parent)
	{
		valueStr = QString::number(initialValue);
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

		this->setStyleSheet(R"(
			 background-color:#e2e6eb
		)");
		QVBoxLayout* layout = new QVBoxLayout();
		QHBoxLayout* layTitleClose = new QHBoxLayout();

		title = new QLabel(label);
		title->setFixedSize(230, 40);
		title->setStyleSheet("font-size: 16px; color: #2c3e50;");
		closeBtn = new ClickableLabel("icon/close-black.png",16);
		closeBtn->setFixedSize(32, 32);
		QObject::connect(closeBtn, &ClickableLabel::clicked, [&]() {
			this->close();
			SpinEditInput* inputSpin = dynamic_cast<SpinEditInput*>(holder);
			if (inputSpin != nullptr)
				inputSpin->clearFocus();
		});
		layTitleClose->addWidget(title);
		layTitleClose->addWidget(closeBtn);
		layout->addLayout(layTitleClose);

		numberBox = new QLabel(QString::number(initialValue) + ".");
		QHBoxLayout* layNumberBox = new QHBoxLayout();
		numberBox->setFixedSize(270, 40);
		layNumberBox->addWidget(numberBox);
		layout->addLayout(layNumberBox);
			numberBox->setStyleSheet(R"(
			background-color: #ffffff;
			border-radius: 2px;
			font-size: 20px;
			padding: 8px 30px 8px 8px;
		)");
		numberBox->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		QHBoxLayout* bottomLayout = new QHBoxLayout();
		QVBoxLayout* BottomLeftLayout = new QVBoxLayout();
		QVBoxLayout* BottomRightLayout = new QVBoxLayout();

		{
			input7 = new QPushButton("7");
			input7->setFixedSize(60, 35);
			input7->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");
			connect(input7, &QPushButton::clicked, [this]() {
					valueStr += "7";
					numberBox->setText(valueStr);
					emit textChanged(valueStr);
			});
			input8 = new QPushButton("8");
			input8->setFixedSize(60, 35);
			input8->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");
			connect(input8, &QPushButton::clicked, [this]() {
					valueStr += "8";
					numberBox->setText(valueStr);
					emit textChanged(valueStr);
				});
			input9 = new QPushButton("9");
			input9->setFixedSize(60, 35);
			input9->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input9, &QPushButton::clicked, [this]() {
					valueStr += "9";
					numberBox->setText(valueStr);
					emit textChanged(valueStr);
			});

			QHBoxLayout* layNumberRow1 = new QHBoxLayout();
			layNumberRow1->addWidget(input7);
			layNumberRow1->addWidget(input8);
			layNumberRow1->addWidget(input9);

			BottomLeftLayout->addLayout(layNumberRow1);
		}

		{
			input4 = new QPushButton("4");
			input4->setFixedSize(60, 35);
			input4->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input4, &QPushButton::clicked, [this]() {
				valueStr += "4";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			input5 = new QPushButton("5");
			input5->setFixedSize(60, 35);
			input5->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input5, &QPushButton::clicked, [this]() {
				valueStr += "5";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			input6 = new QPushButton("6");
			input6->setFixedSize(60, 35);
			input6->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input6, &QPushButton::clicked, [this]() {
				valueStr += "6";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			QHBoxLayout* layNumberRow2 = new QHBoxLayout();
			layNumberRow2 = new QHBoxLayout();
			layNumberRow2->addWidget(input4);
			layNumberRow2->addWidget(input5);
			layNumberRow2->addWidget(input6);
			BottomLeftLayout->addLayout(layNumberRow2);
		}

		{
			input1 = new QPushButton("1");
			input1->setFixedSize(60, 35);
			input1->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input1, &QPushButton::clicked, [this]() {
				valueStr += "1";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			input2 = new QPushButton("2");
			input2->setFixedSize(60, 35);
			input2->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input2, &QPushButton::clicked, [this]() {
				valueStr += "2";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			input3 = new QPushButton("3");
			input3->setFixedSize(60, 35);
			input3->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input3, &QPushButton::clicked, [this]() {
				valueStr += "3";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			QHBoxLayout* layNumberRow3 = new QHBoxLayout();
			layNumberRow3 = new QHBoxLayout();
			layNumberRow3->addWidget(input1);
			layNumberRow3->addWidget(input2);
			layNumberRow3->addWidget(input3);
			BottomLeftLayout->addLayout(layNumberRow3);
		}

		{
			input0 = new QPushButton("0");
			input0->setFixedSize(60, 35);
			input0->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(input0, &QPushButton::clicked, [this]() {
				valueStr += "0";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			inputDot = new QPushButton(".");
			inputDot->setFixedSize(60, 35);
			inputDot->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(inputDot, &QPushButton::clicked, [this]() {
				if (valueStr.contains("."))
					return;
				valueStr += ".";
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			inputSign = new QPushButton("+/-");
			inputSign->setFixedSize(60, 35);
			inputSign->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");

			connect(inputSign, &QPushButton::clicked, [this]() {
				if (valueStr.isEmpty())
					return;

				if (valueStr[0] == '-')
				{
					valueStr.erase(valueStr.begin());
				}
				else
				{
					valueStr.insert(0,"-");
				}
				numberBox->setText(valueStr);
				emit textChanged(valueStr);
			});

			QHBoxLayout* layNumberRow4 = new QHBoxLayout();
			layNumberRow4 = new QHBoxLayout();
			layNumberRow4->addWidget(input0);
			layNumberRow4->addWidget(inputDot);
			layNumberRow4->addWidget(inputSign);
			BottomLeftLayout->addLayout(layNumberRow4);
		}

		{
			inputClear = new QPushButton("C");
			inputClear->setFixedSize(60,90);
			inputClear->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");
			connect(inputClear, &QPushButton::clicked, [this]() {
					valueStr.clear();
					numberBox->setText(valueStr);
					emit textChanged("0");
				});
			inputOK = new QPushButton("OK");
			inputOK->setFixedSize(60,90);
			inputOK->setStyleSheet(R"(
				QPushButton
				{
					background-color: #f4f5f8;
					border: 1px solid rgba(48,65,81,100);
					border-radius: 1px;
					font-weight: bold;
					font-size: 16px; 			
				}
				QPushButton:hover
				{
					background-color: #dce9f1;
					border: 1px solid rgba(44,67,212,100);
				}
			)");
			connect(inputOK, &QPushButton::clicked, [this]() {
					double value = valueStr.toDouble();
					emit ConfirmData(value);
					SpinEditInput* inputSpin = dynamic_cast<SpinEditInput*>(this->holder);
					if (inputSpin != nullptr)
						inputSpin->clearFocus();
					this->hide();
					emit textChanged(valueStr);
				});
			BottomRightLayout->addWidget(inputClear);
			BottomRightLayout->addWidget(inputOK);
		}

		bottomLayout->addLayout(BottomLeftLayout);
		bottomLayout->addLayout(BottomRightLayout);
		layout->addLayout(bottomLayout);

		QWidget* placeholder = new QWidget(this);
		placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		layout->addWidget(placeholder);

		this->setLayout(layout);
		this->setFixedSize(300, 320);
	}

	VirtualKeyBoard::~VirtualKeyBoard()
	{

	}
	void VirtualKeyBoard::SetConfig(const QString& label, double initialValue)
	{
		this->valueStr = QString::number(initialValue);
		this->title->setText(label);
		this->numberBox->setText(QString::number(initialValue) + ".");
	}
	void VirtualKeyBoard::SetValue(double value)
	{
		this->valueStr = QString::number(value);
		this->numberBox->setText(QString::number(value) + ".");
	}
	void VirtualKeyBoard::SetHolder(SpinEditInput* spinBox)
	{
		this->holder = spinBox;
		//connect(this->numberBox)
	}
}