#include "UI/Components/HmiTemplateLabel.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/GlobalPLCVars.h"
#include "NetWork/MessageValidtor.h"
#include <iostream>

HmiTemplateLabel::HmiTemplateLabel(int width, int height,int fontSize)
{
	this->setObjectName("hmiTemplate");
	QFont font("Nirmala UI");
	font.setPixelSize(fontSize);
	this->setFont(font);
	this->setFixedSize(width,height);
	nodeType = ScadaNodeType::SLabel;
}

HmiTemplateLabel::~HmiTemplateLabel()
{
}

void HmiTemplateLabel::UpdateNode()
{
	if (param == nullptr)
	{
		this->BindParam(this->bindTag);
	}
	PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[bindTag]);
	if (plcInfo && plcInfo->updateUI)
	{
		switch (varType)
		{
		case AtomicVarType::BOOL:
		{
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastBool))
			{
				PLC_TYPE_BOOL val = var->GetValue();
				this->setText(prefix + (val ? "True" : "False"));
				lastValue.lastBool = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::WORD:
		{
			AtomicVar<PLC_TYPE_WORD>* var = static_cast<AtomicVar<PLC_TYPE_WORD>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastWord))
			{
				PLC_TYPE_WORD val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastWord = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::DWORD:
		{
			AtomicVar<PLC_TYPE_DWORD>* var = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastDWord))
			{
				PLC_TYPE_DWORD val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastWord = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::LWORD:
		{
			AtomicVar<PLC_TYPE_LWORD>* var = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastLWord))
			{
				PLC_TYPE_LWORD val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastLWord = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::INT:
		{
			AtomicVar<PLC_TYPE_INT>* var = static_cast<AtomicVar<PLC_TYPE_INT>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastInt))
			{
				PLC_TYPE_INT val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastLWord = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::DINT:
		{
			AtomicVar<PLC_TYPE_DINT>* var = static_cast<AtomicVar<PLC_TYPE_DINT>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastDInt))
			{
				PLC_TYPE_DINT val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastDInt = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::LINT:
		{
			AtomicVar<PLC_TYPE_LINT>* var = static_cast<AtomicVar<PLC_TYPE_LINT>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastLInt))
			{
				PLC_TYPE_LINT val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastLInt = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::REAL:
		{
			AtomicVar<PLC_TYPE_REAL>* var = static_cast<AtomicVar<PLC_TYPE_REAL>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastReal))
			{
				PLC_TYPE_REAL val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastReal = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::LREAL:
		{
			AtomicVar<PLC_TYPE_LREAL>* var = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastLReal))
			{
				PLC_TYPE_LREAL val = var->GetValue();
				this->setText(prefix + QString::number(val));
				lastValue.lastLReal = val;
				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
			break;
		}
		case AtomicVarType::STRING:
		{
			AtomicVar<PLC_TYPE_STRING>* var = static_cast<AtomicVar<PLC_TYPE_STRING>*>(param);
			std::string filteredVar(*var);
			filteredVar = trimNonPrintableAscii(filteredVar);
			if (var != NULL)
			{
				this->setText(prefix + QString::fromStdString(filteredVar));
				break;
			}
		}
		}
	}
	else if (plcInfo)
	{
		plcInfo->updateUI = false;
	}
}

void HmiTemplateLabel::showEvent(QShowEvent* event)
{
	ScadaScheduler::GetInstance()->RegisterReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->AddNode(this);
}

void HmiTemplateLabel::hideEvent(QHideEvent* event)
{
	ScadaScheduler::GetInstance()->EraseReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->EraseNode(this);
}
