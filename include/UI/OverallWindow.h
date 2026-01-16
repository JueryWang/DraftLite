#pragma once
#include <QWidget>
#include <Controls/ScadaNode.h>
#include <UI/MenuLayerTop.h>
#include <UI/MainLayer.h>
#include <QTimer>

class OverallWindow : public QWidget
{
	friend class MenuLayerTop;
	friend class ScadaScheduler;
	Q_OBJECT
public:
	OverallWindow();
	~OverallWindow();

public slots:
	void SetShow();
	void SetHide();

private:
	MenuLayerTop* menu;
	MainLayer* mainWindow;

	ScadaNode* monitorIndexPage = nullptr;
	ScadaNode* monitorUploadFTP = nullptr;
	ScadaNode* monitorHeartBeat = nullptr;
	ScadaNode* monitorAutoBusy = nullptr;
	ScadaNode* monitorToolRadius = nullptr;
	ScadaNode* monitorToolDistance = nullptr;

	QTimer heartBeatDetectTimer;
};