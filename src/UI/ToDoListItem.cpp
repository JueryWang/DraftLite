#include "UI/ToDoListItem.h"
#include "UI/TaskListWindow.h"
#include "IO/DxfProcessor.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Graphics/OCS.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateCraftConfig.h"
#include "Controls/GlobalPLCVars.h"
#include "NetWork/FtpClient.h"
#include <QLabel>
#include <QMouseEvent>
#include <QPalette>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QMenu>

ToDoListItemWidget::ToDoListItemWidget(std::shared_ptr<CNCSYS::SketchGPU> _sketch, TaskListWindow* parent) : sketch(_sketch)
{
	fileSource = _sketch->source.c_str();
	this->setAttribute(Qt::WA_DeleteOnClose, false);
	this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	parentListWindow = parent;
	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->setSpacing(10);

	ocsSys = new OCSGPU(sketch);
	ocsSys->SetFitRatio(0.85f);

	sketchImage = g_canvasInstance->GrabImage(sketch.get(), ocsSys, 200, 200);
	std::string content = sketch.get()->ToNcProgram();
	UploadFileToFTP(sketch->source, content);

	checked = new QCheckBox();
	checked->setFixedSize(40, 40);
	checked->setStyleSheet(
		"QCheckBox::indicator {"
		"    width: 20px;"
		"    height: 20px;"
		"    border: 1px solid #777777;" // 关键：手动画个框
		"    border-radius: 3px;"
		"    background-color: white;"
		"}"
		"QCheckBox::indicator:unchecked {"
		"    background-color: white;"
		"}"
		"QCheckBox::indicator:checked {"
		"    background-color: #2196F3;" // 选中时的背景色
		"    image: url(Resources/icon/check.png);"
		"}"
	);
	checked->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	canvas = new QLabel();
	canvas->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	canvas->setPixmap(QPixmap::fromImage(sketchImage));
	checked->hide();

	hlay->setSpacing(0);
	hlay->addWidget(checked);
	hlay->addWidget(canvas);

	QFileInfo fileInfo(QString::fromLocal8Bit(sketch.get()->source.c_str()));
	fileNameLabel = new QLabel(fileInfo.fileName());
	fileNameLabel->setWordWrap(false);
	fileNameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	fileNameLabel->setToolTip(fileNameLabel->text());
	fileNameLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	hlay->addWidget(fileNameLabel);

	planCounterLabel = new QLabel("0");
	QFont font = QFont(global_font_mp["Comic"], 16);
	planCounterLabel->setFont(font);

	//hlay->addSpacing(50);
	hlay->addWidget(planCounterLabel);
	hlay->setContentsMargins(5, 0, 5, 0);
	this->setLayout(hlay);

	connect(checked, &QCheckBox::clicked, [](bool isChecked)
		{
			int checkCount = 0;
			TaskListWindow* window = TaskListWindow::GetInstance();
			for (ToDoListItem* item : window->items)
			{
				if (item->attachedWidget->checked->isChecked())
				{
					checkCount++;
				}
			}

			if (checkCount >= 2)
			{
				window->toolBar->btnItemMoveUp->setEnabled(false);
				window->toolBar->btnItemMoveDown->setEnabled(false);
			}
			else
			{
				window->toolBar->btnItemMoveUp->setEnabled(true);
				window->toolBar->btnItemMoveDown->setEnabled(true);
			}
		});

	ftpUploadSource = extractFTPFileName(fileSource);
}

ToDoListItemWidget::ToDoListItemWidget(const QString& _fileSource, TaskListWindow* parent) : fileSource(_fileSource)
{
	this->setAttribute(Qt::WA_DeleteOnClose, false);
	this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	parentListWindow = parent;
	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->setSpacing(10);
	sketch = std::make_shared<SketchGPU>();
	DXFProcessor processor(sketch);
	processor.SetCompleteCallback([&]()
		{
			for (EntGroup* group : sketch.get()->GetEntityGroups())
			{
				for (EntRingConnection* ring : group->rings)
				{
					if (ring->direction == GeomDirection::CW)
					{
						ring->Reverse();
						ring->direction = GeomDirection::CCW;
					}
				}
			}
			//glm::vec3 firstSecEnd = sketch.get()->GetEntityGroups()[0]->rings[0]->conponents[0]->GetTransformedNodes().back();
			//g_MScontext.XAxisStart = firstSecEnd.x;
			//g_MScontext.YAxisStart = firstSecEnd.y;
			//g_MScontext.ZAxisStart = firstSecEnd.z;
		});
	processor.read(_fileSource.toLocal8Bit().constData());
	ocsSys = new OCSGPU(sketch);
	ocsSys->SetFitRatio(0.85f);

	sketchImage = g_canvasInstance->GrabImage(sketch.get(), ocsSys, 200, 200);
	std::string content = sketch.get()->ToNcProgram();
	UploadFileToFTP(sketch->source, content);

	checked = new QCheckBox();
	checked->setFixedSize(40, 40);
	checked->setStyleSheet(
		"QCheckBox::indicator {"
		"    width: 20px;"
		"    height: 20px;"
		"    border: 1px solid #777777;" // 关键：手动画个框
		"    border-radius: 3px;"
		"    background-color: white;"
		"}"
		"QCheckBox::indicator:unchecked {"
		"    background-color: white;"
		"}"
		"QCheckBox::indicator:checked {"
		"    background-color: #2196F3;" // 选中时的背景色
		"    image: url(Resources/icon/check.png);"
		"}"
	);
	checked->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	canvas = new QLabel();
	canvas->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	canvas->setPixmap(QPixmap::fromImage(sketchImage));
	checked->hide();

	hlay->setSpacing(0);
	hlay->addWidget(checked);
	hlay->addWidget(canvas);

	QFileInfo fileInfo(_fileSource);
	fileNameLabel = new QLabel(fileInfo.fileName());
	fileNameLabel->setWordWrap(false);
	fileNameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	fileNameLabel->setToolTip(fileNameLabel->text());
	fileNameLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	hlay->addWidget(fileNameLabel);

	planCounterLabel = new QLabel("0");
	QFont font = QFont(global_font_mp["Comic"], 16);
	planCounterLabel->setFont(font);

	//hlay->addSpacing(50);
	hlay->addWidget(planCounterLabel);
	hlay->setContentsMargins(5, 0, 5, 0);
	this->setLayout(hlay);

	connect(checked, &QCheckBox::clicked, [](bool isChecked)
		{
			int checkCount = 0;
			TaskListWindow* window = TaskListWindow::GetInstance();
			for (ToDoListItem* item : window->items)
			{
				if (item->attachedWidget->checked->isChecked())
				{
					checkCount++;
				}
			}

			if (checkCount >= 2)
			{
				window->toolBar->btnItemMoveUp->setEnabled(false);
				window->toolBar->btnItemMoveDown->setEnabled(false);
			}
			else
			{
				window->toolBar->btnItemMoveUp->setEnabled(true);
				window->toolBar->btnItemMoveDown->setEnabled(true);
			}
		});

	ftpUploadSource = extractFTPFileName(fileSource);
}

ToDoListItemWidget::~ToDoListItemWidget()
{
	delete ocsSys;
	delete canvas;
	delete fileNameLabel;
	delete planCounterLabel;
	sketch.reset();
}

void ToDoListItemWidget::AddCounter()
{
	this->counter++;
	planCounterLabel->setText(QString::number(counter));
}

void ToDoListItemWidget::mousePressEvent(QMouseEvent* event)
{

}

void ToDoListItemWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		g_canvasInstance->SetScene(sketch, ocsSys);
		parentListWindow->taskLists->setCurrentRow(row);
	}
}

void ToDoListItemWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);

	PLC_TYPE_BOOL isRunning = false;
	ReadPLC_OPCUA(g_ConfigableKeys["AutoBusy"].c_str(), &isRunning, AtomicVarType::BOOL);

	if (!isRunning)
	{
		QAction* actSetCurrent = new QAction("设为当前启动", this);
		menu.addAction(actSetCurrent);
		connect(actSetCurrent, &QAction::triggered, [this]()
			{
				g_canvasInstance->SetScene(sketch, ocsSys);
				parentListWindow->taskLists->setCurrentRow(row);
				parentListWindow->currentRequestNumber = row;
			});
		QAction* actConfigCraft = new QAction(tr("配置工艺"), this);
		menu.addAction(actConfigCraft);
		connect(actConfigCraft, &QAction::triggered, [&]()
			{
				ConfigVariablesPage* page = ConfigVariablesPage::GetInstance();
				page->BindSketch(sketch.get());
				page->show();
				page->raise();
			});
		menu.exec(event->globalPos());
	}
	event->accept();
}

ToDoListItem::ToDoListItem()
{
	this->setSizeHint(QSize(200, 120));
}

ToDoListItem::~ToDoListItem()
{

}

ItemWrapper::ItemWrapper(ToDoListItemWidget* _item) : item(_item)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->addWidget(item);
	this->setLayout(hlay);
}

ItemWrapper::~ItemWrapper()
{
}
