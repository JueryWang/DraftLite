#pragma once
#include <QLabel>

class DashBoard;
class SliderBar;

class DigitalHUD : public QWidget
{
public:
	DigitalHUD(int width,int height);
	~DigitalHUD();

private:
	SliderBar* slider = nullptr;
	DashBoard* board = nullptr;
	QLabel* description = nullptr;
	QLabel* numberArray = nullptr;
};