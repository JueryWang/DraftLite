#include "UI/TaskListWindow.h"
#include "UI/ToDoListItem.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "IO/DxfProcessor.h"
#include "UI/SizeDefines.h"
#include "UI/GLWidget.h"
#include "Graphics/Sketch.h"
#include "Graphics/Canvas.h"
#include "Common/OpenGLContext.h"
#include "ModalEvent/EvCanvasSetNewScene.h"
#include "Common/ProgressInfo.h"
#include "UI/MainLayer.h"
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QApplication>
#include <QList>

TaskListWindow* TaskListWindow::s_instance = nullptr;

TaskListToolBar::TaskListToolBar()
{
	btnSelectAll = new QToolButton(this);
	btnSelectAll->setFocusPolicy(Qt::NoFocus);
	btnSelectAll->setFixedSize(QSize(40, 40));
	btnSelectAll->setIcon(QIcon(ICOPATH(selectAll.png)));
	btnSelectAll->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnSelectAll->setObjectName("hmiTemplate");
	this->addWidget(btnSelectAll);
	btnItemMoveUp = new QToolButton(this);
	btnItemMoveUp->setFocusPolicy(Qt::NoFocus);
	btnItemMoveUp->setFixedSize(QSize(40, 40));
	btnItemMoveUp->setIcon(QIcon(ICOPATH(item_up.png)));
	btnItemMoveUp->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnSelectAll->setObjectName("hmiTemplate");
	this->addWidget(btnItemMoveUp);
	btnItemMoveDown = new QToolButton(this);
	btnItemMoveDown->setFocusPolicy(Qt::NoFocus);
	btnItemMoveDown->setFixedSize(QSize(40, 40));
	btnItemMoveDown->setIcon(QIcon(ICOPATH(item_down.png)));
	btnItemMoveDown->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnItemMoveDown->setObjectName("hmiTemplate");
	this->addWidget(btnItemMoveDown);
	btnItemTop = new QToolButton(this);
	btnItemTop->setFocusPolicy(Qt::NoFocus);
	btnItemTop->setFixedSize(QSize(40, 40));
	btnItemTop->setIcon(QIcon(ICOPATH(top.png)));
	btnItemTop->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnItemTop->setObjectName("hmiTemplate");
	this->addWidget(btnItemTop);
	btnItemBottom = new QToolButton(this);
	btnItemBottom->setFocusPolicy(Qt::NoFocus);
	btnItemBottom->setFixedSize(QSize(40, 40));
	btnItemBottom->setIcon(QIcon(ICOPATH(bottom.png)));
	btnItemBottom->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnItemBottom->setObjectName("hmiTemplate");
	this->addWidget(btnItemBottom);
	btnItemDelete = new QToolButton(this);
	btnItemDelete->setFocusPolicy(Qt::NoFocus);
	btnItemDelete->setFixedSize(QSize(40, 40));
	btnItemDelete->setIcon(QIcon(ICOPATH(trash.png)));
	btnItemDelete->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btnItemDelete->setObjectName("hmiTemplate");
	this->addWidget(btnItemDelete);
}

TaskListToolBar::~TaskListToolBar()
{

}

void TaskListToolBar::EnableTools()
{
	btnSelectAll->setEnabled(true);
	btnItemMoveUp->setEnabled(true);
	btnItemMoveDown->setEnabled(true);
	btnItemTop->setEnabled(true);
	btnItemBottom->setEnabled(true);
	btnItemDelete->setEnabled(true);
}

void TaskListToolBar::DisableTools()
{
	btnSelectAll->setEnabled(false);
	btnItemMoveUp->setEnabled(false);
	btnItemMoveDown->setEnabled(false);
	btnItemTop->setEnabled(false);
	btnItemBottom->setEnabled(false);
	btnItemDelete->setEnabled(false);
}

TaskListWindow* TaskListWindow::GetInstance()
{
	if (TaskListWindow::s_instance == nullptr)
	{
		TaskListWindow::s_instance = new TaskListWindow();
	}

	return TaskListWindow::s_instance;
}

void TaskListWindow::AddTaskItem(ToDoListItem* item, ToDoListItemWidget* itemWidget)
{
	for (ToDoListItem* item : items)
	{
		item->attachedWidget->checked->hide();
	}
	itemWidget->row = items.size();
	taskLists->addItem(item);
	//壳,避免移动时ToDoListItemWidget被自动销毁
	ItemWrapper* wrapper = new ItemWrapper(itemWidget);
	taskLists->setItemWidget(item, wrapper);
	items.push_back(item);
	item->attachedWidget = itemWidget;
}

TaskListWindow::TaskListWindow()
{
	this->setFixedSize(ScreenSizeHintX(TaskWindowPanelWidth_Ratio), ScreenSizeHintY(TaskWindowPanelHeight_Ratio));
	//this->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	toolBar = new TaskListToolBar();

	toolBar->setParent(this);
	toolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	this->setWindowTitle("计划");
	QVBoxLayout* vlay = new QVBoxLayout();
	vlay->setContentsMargins(10, 10, 10, 10);
	addNewBtn = new QPushButton("+");
	connect(addNewBtn, &QPushButton::clicked, this, &TaskListWindow::AddNewPlan);

	addNewBtn->setFixedSize(80, 40);
	vlay->addWidget(addNewBtn, Qt::AlignLeft);

	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->addStretch(1);
	hlay->addWidget(toolBar);
	hlay->addStretch(1);
	taskLists = new QListWidget(this);
	taskLists->setObjectName("hmiTemplate");
	//taskLists->setFixedWidth(250);
	vlay->addWidget(taskLists);
	vlay->addLayout(hlay);
	this->setLayout(vlay);

	connect(toolBar->btnSelectAll, &QPushButton::clicked, this, &TaskListWindow::OnOpenSelect);
	connect(toolBar->btnItemMoveUp, &QPushButton::clicked, this, &TaskListWindow::OnItemMoveUp);
	connect(toolBar->btnItemMoveDown, &QPushButton::clicked, this, &TaskListWindow::OnItemMoveDown);
	connect(toolBar->btnItemTop, &QPushButton::clicked, this, &TaskListWindow::OnItemTop);
	connect(toolBar->btnItemBottom, &QPushButton::clicked, this, &TaskListWindow::OnItemBottom);
	connect(toolBar->btnItemDelete, &QPushButton::clicked, this, &TaskListWindow::OnDeleteItem);
}

void TaskListWindow::AddNewPlan()
{
	if (CheckChangeAvailable())
	{
		QString fileName = QFileDialog::getOpenFileName(
			this,
			("打开文件"),
			QDir::currentPath(),
			("DXF文件(*.dxf);;所有文件(*)")
		);

		if (!fileName.isEmpty())
		{
			ToDoListItem* item = new ToDoListItem();
			ToDoListItemWidget* itemWidget = new ToDoListItemWidget(fileName, this);
			itemWidget->row = this->taskLists->count();
			this->AddTaskItem(item, itemWidget);
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}

bool TaskListWindow::CheckChangeAvailable()
{
	PLC_TYPE_BOOL isRunning = false;
	ReadPLC_OPCUA(g_ConfigableKeys["AutoBusy"].c_str(), &isRunning, AtomicVarType::BOOL);

	return !isRunning;
}

void TaskListWindow::SetCurrent(int number)
{
	currentRequestNumber = number;
	QString fileName = QString::fromLocal8Bit(items[currentRequestNumber]->attachedWidget->sketch->source);
	this->setWindowTitle(QString("当前任务: 第%1项 - %2").arg(number).arg(fileName));
}

std::vector<SketchGPU*> TaskListWindow::GetAllTaskSketches()
{
	std::vector<SketchGPU*> sketches;
	for (ToDoListItem* item : items)
	{
		sketches.push_back(item->attachedWidget->sketch.get());
	}
	return sketches;
}

void TaskListWindow::OnSelectAll()
{
	for (ToDoListItem* item : items)
	{
		item->attachedWidget->checked->setChecked(true);
	}
}
void TaskListWindow::OnOpenSelect()
{
	for (ToDoListItem* item : items)
	{
		item->attachedWidget->checked->show();
	}

	disconnect(toolBar->btnSelectAll, &QPushButton::clicked, this, &TaskListWindow::OnOpenSelect);
	connect(toolBar->btnSelectAll, &QPushButton::clicked, this, &TaskListWindow::OnCloseSelect);
}
void TaskListWindow::OnCloseSelect()
{
	for (ToDoListItem* item : items)
	{
		item->attachedWidget->checked->hide();
	}

	disconnect(toolBar->btnSelectAll, &QPushButton::clicked, this, &TaskListWindow::OnCloseSelect);
	connect(toolBar->btnSelectAll, &QPushButton::clicked, this, &TaskListWindow::OnOpenSelect);
}
void TaskListWindow::OnItemMoveUp()
{
	if (CheckChangeAvailable())
	{
		for (int i = 0; i < items.size(); i++)
		{
			ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
			bool checked = itemWidget->checked->isChecked();
			if (i >= 1 && itemWidget && checked)
			{
				QListWidgetItem* itemA = taskLists->item(items[i]->attachedWidget->row);
				QListWidgetItem* itemB = taskLists->item(items[i - 1]->attachedWidget->row);

				ItemWrapper* wA = (ItemWrapper*)taskLists->itemWidget(itemA);
				ItemWrapper* wB = (ItemWrapper*)taskLists->itemWidget(itemB);

				if (wA) wA->item->setParent(nullptr);
				if (wB) wB->item->setParent(nullptr);

				ItemWrapper* newA = new ItemWrapper(wB->item);
				ItemWrapper* newB = new ItemWrapper(wA->item);

				taskLists->setItemWidget(itemA, newA);
				taskLists->setItemWidget(itemB, newB);
				std::swap(items[i]->attachedWidget->row, items[i - 1]->attachedWidget->row);
				std::swap(items[i], items[i - 1]);
				return;
			}
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}
void TaskListWindow::OnItemMoveDown()
{
	if (CheckChangeAvailable())
	{
		for (int i = 0; i < items.size(); i++)
		{
			ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
			bool checked = itemWidget->checked->isChecked();
			if (i < (items.size() - 1) && itemWidget && checked)
			{
				QListWidgetItem* itemA = taskLists->item(items[i]->attachedWidget->row);
				QListWidgetItem* itemB = taskLists->item(items[i + 1]->attachedWidget->row);

				ItemWrapper* wA = (ItemWrapper*)taskLists->itemWidget(itemA);
				ItemWrapper* wB = (ItemWrapper*)taskLists->itemWidget(itemB);

				if (wA) wA->item->setParent(nullptr);
				if (wB) wB->item->setParent(nullptr);

				ItemWrapper* newA = new ItemWrapper(wB->item);
				ItemWrapper* newB = new ItemWrapper(wA->item);

				taskLists->setItemWidget(itemA, newA);
				taskLists->setItemWidget(itemB, newB);
				std::swap(items[i]->attachedWidget->row, items[i + 1]->attachedWidget->row);
				std::swap(items[i], items[i + 1]);
				return;
			}
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}
void TaskListWindow::OnItemTop()
{
	if (CheckChangeAvailable())
	{
		if (items.size())
		{
			for (int i = (items.size() - 1); i >= 0; i--)
			{
				ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
				bool checked = itemWidget->checked->isChecked();
				if (i != 0 && itemWidget && checked)
				{
					QListWidgetItem* itemA = taskLists->item(items[i]->attachedWidget->row);
					QListWidgetItem* itemB = taskLists->item(items[0]->attachedWidget->row);

					ItemWrapper* wA = (ItemWrapper*)taskLists->itemWidget(itemA);
					ItemWrapper* wB = (ItemWrapper*)taskLists->itemWidget(itemB);

					if (wA) wA->item->setParent(nullptr);
					if (wB) wB->item->setParent(nullptr);

					ItemWrapper* newA = new ItemWrapper(wB->item);
					ItemWrapper* newB = new ItemWrapper(wA->item);

					taskLists->setItemWidget(itemA, newA);
					taskLists->setItemWidget(itemB, newB);
					std::swap(items[i]->attachedWidget->row, items[0]->attachedWidget->row);
					std::swap(items[i], items[0]);
				}
			}
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}
void TaskListWindow::OnItemBottom()
{
	if (CheckChangeAvailable())
	{
		if (items.size())
		{
			for (int i = 0; i < items.size() - 1; i++)
			{
				ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
				bool checked = itemWidget->checked->isChecked();
				if (i != (items.size() - 1) && itemWidget && checked)
				{
					QListWidgetItem* itemA = taskLists->item(items[i]->attachedWidget->row);
					QListWidgetItem* itemB = taskLists->item(items.back()->attachedWidget->row);

					ItemWrapper* wA = (ItemWrapper*)taskLists->itemWidget(itemA);
					ItemWrapper* wB = (ItemWrapper*)taskLists->itemWidget(itemB);

					if (wA) wA->item->setParent(nullptr);
					if (wB) wB->item->setParent(nullptr);

					ItemWrapper* newA = new ItemWrapper(wB->item);
					ItemWrapper* newB = new ItemWrapper(wA->item);

					taskLists->setItemWidget(itemA, newA);
					taskLists->setItemWidget(itemB, newB);
					std::swap(items[i]->attachedWidget->row, items.back()->attachedWidget->row);
					std::swap(items[i], items.back());
				}
			}
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}
void TaskListWindow::OnDeleteItem()
{
	if (CheckChangeAvailable())
	{
		for (int i = 0; i < items.size(); i++)
		{
			ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
			bool checked = itemWidget->checked->isChecked();
			std::cout << items[i]->attachedWidget->ocsSys << std::endl;
			if (itemWidget && checked)
			{
				QListWidgetItem* listItem = taskLists->takeItem(taskLists->row(items[i]));
				delete itemWidget;
				delete listItem;
				std::swap(items[i], items[items.size() - 1]);
				items.erase(items.end() - 1);
				i--;
			}
		}

		for (int i = 0; i < items.size(); i++)
		{
			items[i]->attachedWidget->row = i;
		}
		if (items.size() > 0)
		{
			EvCanvasSetNewScene* evSetScene = new EvCanvasSetNewScene(items[0]->attachedWidget->sketch, items[0]->attachedWidget->ocsSys);
			QApplication::postEvent(g_canvasInstance->GetFrontWidget(), evSetScene, Qt::HighEventPriority);
		}
		else
		{
			EvCanvasSetNewScene* evSetScene = new EvCanvasSetNewScene(g_mainWindow->GetSketchShared(), g_mainWindow->GetSketch()->attachedOCS);
			QApplication::postEvent(g_canvasInstance->GetFrontWidget(), evSetScene, Qt::HighEventPriority);
		}
	}
	else
	{
		QMessageBox::critical(NULL, tr("错误"), tr("轴启动中无法再变更计划!"));
	}
}

void TaskListWindow::ClearItems()
{
	for (int i = 0; i < items.size(); i++)
	{
		ToDoListItemWidget* itemWidget = items[i]->attachedWidget;
		if (itemWidget)
		{
			QListWidgetItem* listItem = taskLists->takeItem(taskLists->row(items[i]));
			delete itemWidget;
			delete listItem;
		}
	}
	items.clear();
}

void TaskListWindow::OnCurrentItemChanged()
{

}

bool TaskListWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	return QWidget::nativeEvent(eventType, message, result);
}

TaskListWindow::~TaskListWindow()
{
	delete addNewBtn;
	delete taskLists;
	delete toolBar;
}
