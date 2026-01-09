#include "UI/Components/HmiTemplateButton.h"
#include "Controls/GlobalPLCVars.h"
#include <Controls/ScadaScheduler.h>

HmiTemplateButton::HmiTemplateButton(int width, int height,int fontsize)
{
	this->setObjectName("hmiTemplate");
	QFont font("Nirmala UI");
	font.setPixelSize(fontsize);
	this->setFont(font);
	this->setFixedSize(width, height);
	nodeType = ScadaNodeType::SPubtton;
}

HmiTemplateButton::~HmiTemplateButton()
{
}

void HmiTemplateButton::UpdateNode()
{
	if (param == nullptr)
	{
		this->BindParam(this->bindTag);
	}

	PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[bindTag]);
	if (plcInfo && plcInfo->updateUI)
	{
		if (varType == AtomicVarType::BOOL)
		{
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			if (var != NULL && !(var->GetValue() == lastValue.lastBool))
			{
				PLC_TYPE_BOOL val = var->GetValue();
				lastValue.lastBool = val;

				if (valueChangedCallback)
				{
					valueChangedCallback(var);
				}
			}
		}
	}
}

void HmiTemplateButton::SetPressingStyle()
{
	this->setStyleSheet(ButtonPressingStyle);
}

void HmiTemplateButton::SetReleasingStyle()
{
	this->setStyleSheet("");
}

void HmiTemplateButton::showEvent(QShowEvent* event)
{
	ScadaScheduler::GetInstance()->RegisterReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->AddNode(this);
}

void HmiTemplateButton::hideEvent(QHideEvent* event)
{
	ScadaScheduler::GetInstance()->EraseReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->EraseNode(this);
}
