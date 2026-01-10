#pragma once

#include <memory>
#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QListWidgetItem>
#include <QContextMenuEvent>
#include <QCheckBox>
#include <QProxyStyle>

namespace CNCSYS
{
	class SketchGPU;
	class OCSGPU;
}
using namespace CNCSYS;
class TaskListWindow;

class ToDoListItemWidget : public QWidget
{
public:
	ToDoListItemWidget(const QString& fileSource,TaskListWindow* parent = nullptr);
	~ToDoListItemWidget();
	void AddCounter();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

public:
	std::shared_ptr<SketchGPU> sketch;
	OCSGPU* ocsSys;
	TaskListWindow* parentListWindow = nullptr;

	QLabel* canvas = nullptr;
	QCheckBox* checked = nullptr;
	QLabel* fileNameLabel = nullptr;
	QLabel* planCounterLabel = nullptr;
	QImage sketchImage;
	QString fileSource;
	std::string ftpUploadSource;
	int row;
	int counter = 0;
};

class ItemWrapper : public QWidget
{
public:
	ItemWrapper(ToDoListItemWidget* _item);
	~ItemWrapper();

	operator ToDoListItemWidget* () {
		return this->item;
	}

public:
	ToDoListItemWidget* item;
};

class ToDoListItem : public QListWidgetItem
{
public:
	ToDoListItem();
	~ToDoListItem();

public:
	ToDoListItemWidget* attachedWidget = nullptr;
};