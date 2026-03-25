#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include "Common/Program.h"

namespace CNCSYS
{
	class SketchGPU;
}

class SketchInfoPanel : public QWidget
{
	Q_OBJECT
public:
	SketchInfoPanel(QWidget* parent = nullptr);
	~SketchInfoPanel();
	void updateStats(CNCSYS::SketchGPU* sketch);
	void setupUI();

private slots:
	void CommConnect();
	void CommDisconnect();

private:
	void applyStyle();

public:
	QGroupBox* groupBox;
	QLabel* labelEntities, * labelContours, * labelTotalPath, * labelIdlePath, *labelIdleRatio,*labelDimension, *labelSource;
	QPushButton* btnOpenComm;
};

