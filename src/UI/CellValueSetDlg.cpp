#include "Controls/GlobalPLCVars.h"
#include "UI/CellValueSetDlg.h"
#include "Controls/ScadaScheduler.h"
#include "NetWork/MessageValidtor.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

CellValueSetDlg::CellValueSetDlg()
{
	this->setWindowFlags(Qt::Tool);

	QVBoxLayout* vlay = new QVBoxLayout();
	QHBoxLayout* hlay = new QHBoxLayout();
	QLabel* hint1 = new QLabel("当前值:");
	hlay->addWidget(hint1);
	labelCurValue = new HmiTemplateLabel(80,30);
	hlay->addWidget(labelCurValue);
	QLabel* hint2 = new QLabel("设置新值:");
	hlay->addWidget(hint2);
	inputEdit = new QLineEdit();
	hlay->addWidget(inputEdit);
	
	QHBoxLayout* btnLayout = new QHBoxLayout();
	btnLayout->setContentsMargins(5, 5, 10, 5);
	QPushButton* btnConfirm = new QPushButton("确定");
	QObject::connect(btnConfirm, &QPushButton::clicked, [&]()
	{
		switch (currentHandleType)
		{
			case AtomicVarType::BOOL:
			{
				PLC_TYPE_BOOL newValue;
				if (inputEdit->text() == "0")
				{
					newValue = false;
				}
				else if (inputEdit->text() == "1")
				{
					newValue = true;
				}
				else if (inputEdit->text().toLower() == "false")
				{
					newValue = false;
				}
				else if (inputEdit->text().toLower() == "true")
				{
					newValue = true;
				}
				WritePLC_OPCUA(bindTag.c_str(), &newValue,AtomicVarType::BOOL);
				break;
			}
			case AtomicVarType::WORD:
			{
				PLC_TYPE_WORD newValue;
				if (stringToUint16(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::WORD);
				}
				break;
			}
			case AtomicVarType::DWORD:
			{
				PLC_TYPE_DWORD newValue;
				if (stringToUint32(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(), &newValue, AtomicVarType::DWORD);
				}
				break;
			}
			case AtomicVarType::LWORD:
			{
				PLC_TYPE_LWORD newValue;
				if (stringToUint64(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::LWORD);
				}
				break;
			}
			case AtomicVarType::INT:
			{
				PLC_TYPE_INT newValue;
				if (stringToInt16(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::INT);
				}
				break;
			}
			case AtomicVarType::DINT:
			{
				PLC_TYPE_DINT newValue;
				if (stringToInt32(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(), &newValue, AtomicVarType::DINT);
				}
				break;
			}
			case AtomicVarType::LINT:
			{
				PLC_TYPE_LINT newValue;
				if (stringToInt64(inputEdit->text(), newValue))
				{
					WritePLC_OPCUA(bindTag.c_str(), &newValue, AtomicVarType::LINT);
				}
				break;
			}
			case AtomicVarType::REAL:
			{
				PLC_TYPE_REAL newValue;
				bool ok;
				newValue = inputEdit->text().toFloat(&ok);
				WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::REAL);
				break;
			}
			case AtomicVarType::LREAL:
			{
				PLC_TYPE_LREAL newValue;
				bool ok;
				newValue = inputEdit->text().toDouble(&ok);
				WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::LREAL);
				break;
			}
			case AtomicVarType::STRING:
			{
				QByteArray byteArr = inputEdit->text().toLocal8Bit();
				PLC_TYPE_STRING newValue = byteArr.data();
				WritePLC_OPCUA(bindTag.c_str(),&newValue,AtomicVarType::STRING);
				break;
			}
		}
		this->close();
	});

	QPushButton* btnCancel = new QPushButton("取消");
	QObject::connect(btnCancel, &QPushButton::clicked, [&]() {
			this->close();
		}
	);
	QSpacerItem* placeHolder = new QSpacerItem(70, 25, QSizePolicy::Expanding);
	btnLayout->addSpacerItem(placeHolder);
	btnLayout->addWidget(btnConfirm);
	btnLayout->addWidget(btnCancel);

	vlay->addLayout(hlay);
	vlay->addLayout(btnLayout);

	this->setLayout(vlay);
}

CellValueSetDlg::~CellValueSetDlg()
{
	delete labelCurValue;
}

void CellValueSetDlg::SetValueTag(const std::string& tag)
{
	labelCurValue->BindParam(tag);
	bindTag = tag;
}

void CellValueSetDlg::closeEvent(QCloseEvent* event)
{
	ScadaScheduler::GetInstance()->EraseNode(labelCurValue);
}

void CellValueSetDlg::showEvent(QShowEvent* event)
{
	inputEdit->setText("");
	ScadaScheduler::GetInstance()->AddNode(labelCurValue);
}
