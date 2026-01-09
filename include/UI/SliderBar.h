#pragma once

#include <QSlider>

class SliderBar : public QSlider
{
public:
	SliderBar(int minValue, int maxValue);
	~SliderBar();

private:
	int min;
	int max;
};