#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
namespace CNCSYS
{
	class SketchGPU;
}
class SketchInfoPanel : public QWidget
{
public:
	SketchInfoPanel(QWidget* parent = nullptr);
	~SketchInfoPanel();
	void updateStats(CNCSYS::SketchGPU* sketch);
	void setupUI();
private:
	void applyStyle();

private:
	QGroupBox* groupBox;
	QLabel* labelEntities, * labelContours, * labelTotalPath, * labelIdlePath, *labelIdleRatio,*labelDimension, *labelSource;
};