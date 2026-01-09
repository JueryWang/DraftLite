#pragma once

#include "Controls/ScadaNode.h"
#include <open62541/types.h>
#include <QString>
#include <QTimer>
#include <QColor>
#include <QMap>
#include <QTableWidget>
#include <vector>
#include <string>
#include <mutex>

class SearchItemDialog;
class CellValueSetDlg;

class HmiTemplateTableCellItem : public ScadaNode, public QTableWidgetItem
{
	friend class ScadaScheduler;
public:
	HmiTemplateTableCellItem(const std::string& bindTag, AtomicVarType type);
	~HmiTemplateTableCellItem();
	void SetUpdateRate(int rateInMs);
	virtual void UpdateNode() override;

public:
	std::string tag;
	AtomicVarType valueType;
	QTableWidgetItem* timmerCellItem = nullptr;

private: 
	void UpdateCell(const QString& oldValue, const QString& newValue);

private:
	std::atomic<int> updateInterval = 50;
	
	QColor defaultBgColor;
};

class HmiTemplateTableMonitor : public QTableWidget
{
	Q_OBJECT
	friend class ScadaScheduler;
public:
	HmiTemplateTableMonitor(const std::vector<std::string>& tagsInMonitor);
	~HmiTemplateTableMonitor();
	void SetUpdateRate(int rateInMs);
	void CreateTableFromPLCVarMap();
	void AddMonitorItem(const std::string& newItemTag);
	void ScaleToFit();
	void ClearItems();
	void moveRowToTop(const QString& tag);

signals:
	void sizeChanged(const QSize& newSize);

protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	void InsertItem(int row, const std::string& tag);
	void CreateHeader();
	QTableWidgetItem* GetItemAtEvent(QMouseEvent* event);

private:
	static CellValueSetDlg* valueSetDialog;
	std::vector<std::string> monitorTags;
	std::mutex mtx;

	QMap<std::string, HmiTemplateTableCellItem*> monitorItemMap;
	QStringList headerLabels;
	QStringList searchKeys;

	SearchItemDialog* searchDlg = nullptr;
};
