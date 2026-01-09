#include "UI/Components/HmiTemplateSliderBar.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/GlobalPLCVars.h"

HmiTemplateSlider::HmiTemplateSlider(int width, int height)
{
	this->setObjectName("hmiTemplate");
	this->setFixedSize(width, height);
	nodeType = ScadaNodeType::SSliderBar;
}

HmiTemplateSlider::~HmiTemplateSlider()
{
}

void HmiTemplateSlider::UpdateNode()
{
}

void HmiTemplateSlider::showEvent(QShowEvent* event)
{
	ScadaScheduler::GetInstance()->RegisterReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->AddNode(this);
}

void HmiTemplateSlider::hideEvent(QHideEvent* event)
{
	ScadaScheduler::GetInstance()->EraseReadBackVarKey(this->bindTag);
	ScadaScheduler::GetInstance()->EraseNode(this);
}
