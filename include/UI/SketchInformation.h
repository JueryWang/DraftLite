#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>

class SketchInfoPanel : public QWidget
{
public:
	SketchInfoPanel(QWidget* parent = nullptr);
	~SketchInfoPanel();
	void updateStats(int entities, int contours, double totalLen, double idleLen, const QSize& size);
	void setupUI();
private:
	void applyStyle();

private:
	QGroupBox* groupBox;
	QLabel* labelEntities, * labelContours, * labelTotalPath, * labelIdlePath, * labelIdleRatio,*labelDimension;
};