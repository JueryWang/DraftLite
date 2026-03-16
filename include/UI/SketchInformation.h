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

class StatusBar : public QWidget
{
	Q_OBJECT
public:
	enum Status {
		Idle,
		Running,
		Pause,
		Finish,
		Error
	};

	explicit StatusBar(QWidget* parent = nullptr);
	void setStatus(Status status,const QString &text);

public:
	QLabel* m_label;
	Status m_currentStatus;
};

class SketchInfoPanel : public QWidget
{
public:
	SketchInfoPanel(QWidget* parent = nullptr);
	~SketchInfoPanel();
	void updateStats(CNCSYS::SketchGPU* sketch);
	void setupUI();
private:
	void applyStyle();

public:
	QGroupBox* groupBox;
	QLabel* labelEntities, * labelContours, * labelTotalPath, * labelIdlePath, *labelIdleRatio,*labelDimension, *labelSource;
	StatusBar* statusInfo;
};

