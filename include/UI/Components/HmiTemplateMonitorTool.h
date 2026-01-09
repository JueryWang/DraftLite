#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QSettings>
#include <QRadioButton>
#include <QPushButton>
#include "UI/Components/HmiTemplateTableMonitor.h"

class HmiTemplateMonitorTool;

class HmiTemplateConfigVarSettings : public QWidget
{
public:
	HmiTemplateConfigVarSettings(QSettings* config);
	~HmiTemplateConfigVarSettings();

private:
	QSettings* setting;
	QRadioButton* protoOPCUARadio = nullptr;
	QRadioButton* protoModbusTCPRadio = nullptr;
	QRadioButton* protoModbusRTURadio = nullptr;
	QLineEdit* urlEdit = nullptr;
	QLineEdit* addressEdit = nullptr;
	QPushButton* btnConfirm = nullptr;
	QPushButton* btnCancel = nullptr;
};

class HmiTemplateMonitorTool : public QMainWindow
{
public:
	HmiTemplateMonitorTool();
	~HmiTemplateMonitorTool();

public slots:
	void OnConfigVariant();
	void OnConnectOPCUA();
	void OnExportExcel();
	void OnImportExcel();
	void OnTableSizeChanged(const QSize& size);

private:
	void CreateMenu();

public:
	HmiTemplateConfigVarSettings* configPanel = nullptr;
	HmiTemplateTableMonitor* table;
	QSettings* setting;
};