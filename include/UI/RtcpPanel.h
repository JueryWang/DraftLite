#pragma once
#include <QWidget>
#include <QLabel>
#include "UI/DashBoard.h"
#include "UI/SliderBar.h"

class RTCPPanel : public QWidget
{
	Q_OBJECT
public:
	RTCPPanel(QWidget* parent = nullptr);
	~RTCPPanel();

private:


	QLabel* IconX = nullptr;
	QLabel* CurValueX = nullptr;
	QLabel* TargetValueX = nullptr;
	int minX;
	int maxX;

	QLabel* IconY = nullptr;
	QLabel* CurValueY = nullptr;
	QLabel* TargetValueY = nullptr;
	int minY;
	int maxY;

	QLabel* IconZ = nullptr;
	QLabel* CurValueZ = nullptr;
	QLabel* TargetValueZ = nullptr;
	int minZ;
	int maxZ;

	QLabel* IconA = nullptr;
	QLabel* CurValueA = nullptr;
	QLabel* TargetValueA = nullptr;
	int minA;
	int maxA;
};
