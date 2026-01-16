#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QMap>
#include <QMouseEvent>
#include <vector>
#include "UI/ToDoListItem.h"

using namespace CNCSYS;

class TaskListToolBar : public QToolBar
{
	friend class TaskListWindow;
	friend class ToDoListItemWidget;
public:
	TaskListToolBar();
	~TaskListToolBar();
	void EnableTools();
	void DisableTools();

private:
	QToolButton* btnSelectAll = nullptr;
	QToolButton* btnItemMoveUp = nullptr;
	QToolButton* btnItemMoveDown = nullptr;
	QToolButton* btnItemTop = nullptr;
	QToolButton* btnItemBottom = nullptr;
	QToolButton* btnItemDelete = nullptr;
};

class TaskListWindow : public QWidget
{
	Q_OBJECT
		friend class TaskListToolBar;
	friend class ToDoListItemWidget;
public:
	static TaskListWindow* GetInstance();

	void AddTaskItem(ToDoListItem* item, ToDoListItemWidget* itemWidget);
	void ClearItems();
	void AddNewPlan();
	bool CheckChangeAvailable();
	void SetCurrent(int number);
	std::vector <SketchGPU*> GetAllTaskSketches();
	ToDoListItemWidget* CurrentItemWidget() const { return items[currentRequestNumber]->attachedWidget; }

public slots:
	void OnSelectAll();
	void OnOpenSelect();
	void OnCloseSelect();
	void OnItemMoveUp();
	void OnItemMoveDown();
	void OnItemTop();
	void OnItemBottom();
	void OnDeleteItem();
	void OnCurrentItemChanged();

protected:
	virtual bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private:
	TaskListWindow();
	~TaskListWindow();

private:
	static TaskListWindow* s_instance;

public:
	QPushButton* addNewBtn = nullptr;
	QListWidget* taskLists = nullptr;
	TaskListToolBar* toolBar = nullptr;
	int currentRequestNumber = 0;

	std::vector<ToDoListItem*> items;
};