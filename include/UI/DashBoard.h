#pragma once

#include<QWidget>
#include<QSlider>
#include<QPainter>

class DashBoard : public QWidget
{
public:
	DashBoard(int radius,int minValue,int maxValue);
	~DashBoard();

	void SetValue(int value);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	void drawBg(QPainter* painter);
	void drawDial(QPainter* painter);
	void drawScaleNum(QPainter* painter);
	void drawIndicator(QPainter* painter);
	void drawText(QPainter* painter);

private:
	int radius;
	int startAngle = 40;
	int minVal;
	int maxVal;
	int currentValue = 0;
};