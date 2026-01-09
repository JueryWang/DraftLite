#include "UI/SliderBar.h"

SliderBar::SliderBar(int minValue, int maxValue) : min(minValue),max(maxValue)
{
	this->setOrientation(Qt::Horizontal);
}

SliderBar::~SliderBar()
{

}
