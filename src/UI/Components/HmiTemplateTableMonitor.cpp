#include "UI/Components/HmiTemplateTableMonitor.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateMsgBox.h"
#include "UI/CellValueSetDlg.h"
#include "UI/SearchItemDlg.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include <QMenu>
#include <QMouseEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QDateTime>
#include <algorithm>

#define COL_COUNT 6
using namespace std::chrono;
CellValueSetDlg* HmiTemplateTableMonitor::valueSetDialog = nullptr;

HmiTemplateTableCellItem::HmiTemplateTableCellItem(const std::string& bindTag, AtomicVarType type) : tag(bindTag), valueType(type)
{
	defaultBgColor = Qt::white;
}

HmiTemplateTableCellItem::~HmiTemplateTableCellItem()
{
	ScadaScheduler::GetInstance()->EraseNode(this);
}

void HmiTemplateTableCellItem::SetUpdateRate(int rateInMs)
{
	updateInterval.store(rateInMs, std::memory_order_release);
}

void HmiTemplateTableCellItem::UpdateNode()
{
	if (param != nullptr)
	{
		PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[bindTag]);
		if (plcInfo && plcInfo->updateUI)
		{
			switch (varType)
			{
			case AtomicVarType::BOOL:
			{
				AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_BOOL val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = val ? "True" : "False";
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::WORD:
			{
				AtomicVar<PLC_TYPE_WORD>* var = static_cast<AtomicVar<PLC_TYPE_WORD>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_WORD val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::DWORD:
			{
				AtomicVar<PLC_TYPE_DWORD>* var = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_DWORD val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::LWORD:
			{
				AtomicVar<PLC_TYPE_LWORD>* var = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_LWORD val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::INT:
			{
				AtomicVar<PLC_TYPE_INT>* var = static_cast<AtomicVar<PLC_TYPE_INT>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_INT val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::DINT:
			{
				AtomicVar<PLC_TYPE_DINT>* var = static_cast<AtomicVar<PLC_TYPE_DINT>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_DINT val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::LINT:
			{
				AtomicVar<PLC_TYPE_LINT>* var = static_cast<AtomicVar<PLC_TYPE_LINT>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_LINT val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::REAL:
			{
				AtomicVar<PLC_TYPE_REAL>* var = static_cast<AtomicVar<PLC_TYPE_REAL>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_REAL val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::LREAL:
			{
				AtomicVar<PLC_TYPE_LREAL>* var = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(param);
				if (var != NULL)
				{
					PLC_TYPE_LREAL val = var->GetValue();
					QString oldValue = this->text();
					QString newValue = QString::number(val);
					UpdateCell(oldValue, newValue);
				}
				break;
			}
			case AtomicVarType::STRING:
			{
				AtomicVar<PLC_TYPE_STRING>* var = static_cast<AtomicVar<PLC_TYPE_STRING>*>(param);
				if (var != NULL)
				{
					QString oldValue = this->text();
					QString newValue = QString::fromLocal8Bit(var->GetValue());
					UpdateCell(oldValue, newValue);
				}
			}
			}
		}
		else if (plcInfo)
		{
			plcInfo->updateUI = false;
		}
	}
}

void HmiTemplateTableCellItem::UpdateCell(const QString& oldValue, const QString& newValue)
{
	if (oldValue != newValue)
	{
		this->setBackground(VariantChangeWarning);
		this->setText(newValue);
		QDateTime currentTime = QDateTime::currentDateTime();
		this->timmerCellItem->setText(currentTime.toString("yyyy-MM-dd HH:mm:ss"));
	}
	else
	{
		this->setBackground(defaultBgColor);
	}
}

HmiTemplateTableMonitor::HmiTemplateTableMonitor(const std::vector<std::string>& tagsInMonitor) : monitorTags(tagsInMonitor)
{
	//tag type addressDescription protocol curValue
	setAttribute(Qt::WA_DeleteOnClose, true);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	int rowCount = monitorTags.size();
	int colCount = COL_COUNT;

	this->setRowCount(rowCount);
	this->setColumnCount(colCount);
	CreateHeader();

	for (int i = 0; i < rowCount; i++)
	{
		InsertItem(i, monitorTags[i]);
	}

	this->ScaleToFit();
	searchDlg = new SearchItemDialog(searchKeys);
	auto searchCb = [this](const QString& key)
		{
			for (int row = 0; row < this->rowCount(); ++row)
			{
				QTableWidgetItem* item = this->item(row, 0);
				if (item && item->text() == key)
				{
					this->setCurrentCell(row, 0);
					item->setSelected(true);
					this->scrollToItem(item, QAbstractItemView::EnsureVisible);
				}
			}
		};
	searchDlg->SetCallback(searchCb);

	HmiTemplateTableMonitor::valueSetDialog = new CellValueSetDlg();
	HmiTemplateTableMonitor::valueSetDialog->hide();
}

HmiTemplateTableMonitor::~HmiTemplateTableMonitor()
{
	for (auto it = monitorItemMap.begin(); it != monitorItemMap.end(); ++it) {
		HmiTemplateTableCellItem* item = it.value(); // 获取当前迭代器指向的value
		delete item;
	}
	delete searchDlg;
	HmiTemplateTableMonitor::valueSetDialog->hide();
}

void HmiTemplateTableMonitor::CreateTableFromPLCVarMap()
{
	ClearItems();
	CreateHeader();

	searchKeys.clear();
	std::vector<std::string> tags;
	for (auto& pair : g_PLCVariables)
	{
		tags.push_back(pair.first);
	}
	monitorTags = tags;

	int rowCount = tags.size();
	int colCount = COL_COUNT;
	this->setRowCount(rowCount);
	this->setColumnCount(colCount);

	CreateHeader();

	for (int i = 0; i < rowCount; i++)
	{
		InsertItem(i, monitorTags[i]);
	}
	this->ScaleToFit();

	searchDlg->SetNewKeys(searchKeys);
}

void HmiTemplateTableMonitor::AddMonitorItem(const std::string& newItemTag)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto find = std::find(monitorTags.begin(), monitorTags.end(), newItemTag);
	if (find != monitorTags.end())
	{
		monitorTags.push_back(newItemTag);
		int row = this->rowCount();

		// 在末尾插入新行
		this->insertRow(row);
		InsertItem(row, newItemTag);
		searchDlg->SetNewKeys(searchKeys);
	}
}

void HmiTemplateTableMonitor::ScaleToFit()
{
	this->resizeColumnsToContents();
	this->resizeRowsToContents();

	// 更精确的尺寸计算
	int totalWidth = this->verticalHeader()->width(); // 垂直表头宽度
	for (int col = 0; col < this->columnCount(); ++col) {
		totalWidth += this->columnWidth(col);
	}
	// 考虑滚动条和边框
	totalWidth += this->verticalScrollBar()->sizeHint().width()
		+ 2 * this->frameWidth();

	int totalHeight = this->horizontalHeader()->height(); // 水平表头高度
	for (int row = 0; row < 30; ++row) {
		totalHeight += this->rowHeight(row);
	}
	totalHeight += this->horizontalScrollBar()->sizeHint().height()
		+ 2 * this->frameWidth();

	// 设置表格大小
	this->setMinimumSize(totalWidth, totalHeight);

	this->adjustSize();
	emit sizeChanged(QSize(totalWidth + 60, totalHeight));
}

void HmiTemplateTableMonitor::ClearItems()
{
	for (int row = 0; row < this->rowCount(); ++row)
	{
		QTableWidgetItem* item1 = this->takeItem(row, 0);
		QTableWidgetItem* item2 = this->takeItem(row, 1);
		QTableWidgetItem* item3 = this->takeItem(row, 2);
		QTableWidgetItem* item4 = this->takeItem(row, 3);
		HmiTemplateTableCellItem* item5 = (HmiTemplateTableCellItem*)this->takeItem(row, 4);
		QTableWidgetItem* item6 = this->takeItem(row, 5);
		delete item1;
		delete item2;
		delete item3;
		delete item4;
		delete item5;
		delete item6;
	}
	this->clear();
	monitorTags.clear();
	monitorItemMap.clear();
	headerLabels.clear();
	searchKeys.clear();
}

void HmiTemplateTableMonitor::moveRowToTop(const QString& tag)
{
	int row = searchKeys.indexOf(tag);
	QAbstractItemModel* model = this->model();
	// 移动行：从targetRow移动到0行（行移动后，原行位置会被后续行填充）
	model->moveRow(QModelIndex(), row, QModelIndex(), 0);
}

void HmiTemplateTableMonitor::mouseDoubleClickEvent(QMouseEvent* event)
{
	HmiTemplateTableCellItem* cellItem = dynamic_cast<HmiTemplateTableCellItem*>(GetItemAtEvent(event));
	if (cellItem)
	{
		HmiTemplateTableMonitor::valueSetDialog->currentHandleType = cellItem->valueType;
		HmiTemplateTableMonitor::valueSetDialog->SetValueTag(cellItem->tag);
		HmiTemplateTableMonitor::valueSetDialog->setWindowTitle(QString("设置新值:%1").arg(QString::fromStdString(cellItem->tag)));
		HmiTemplateTableMonitor::valueSetDialog->show();
		HmiTemplateTableMonitor::valueSetDialog->raise();
	}
}

void HmiTemplateTableMonitor::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);

	QTableWidgetItem* item = itemAt(event->pos());
	QAction* hideAction = menu.addAction("隐藏列");

	QObject::connect(hideAction, &QAction::triggered, [&]() {
		this->setColumnHidden(item->column(), true);
		this->ScaleToFit();
		});

	QAction* showAllAction = menu.addAction("展开所有列");
	QObject::connect(showAllAction, &QAction::triggered, [&]() {
		for (int i = 0; i < COL_COUNT; i++)
		{
			this->setColumnHidden(i, false);
		}
		this->ScaleToFit();
		});

	QAction* searchAction = menu.addAction("查找");
	QObject::connect(searchAction, &QAction::triggered, [&]() {
		searchDlg->show();
		searchDlg->raise();
		});

	menu.exec(event->globalPos());
}

void HmiTemplateTableMonitor::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers() & Qt::ControlModifier &&
		event->key() == Qt::Key_F) {
		if (searchDlg)
		{
			searchDlg->show();
			QRect mainRect = this->geometry();
			QRect dlgRect = searchDlg->geometry();

			int x = mainRect.x() + (mainRect.width() - dlgRect.width()) / 2;
			int y = mainRect.y() + (mainRect.height() - dlgRect.height()) / 2;

			searchDlg->move(x, y);
		}
	}
}

void HmiTemplateTableMonitor::InsertItem(int row, const std::string& tag)
{
	//column tag
	{
		QTableWidgetItem* item1 = new QTableWidgetItem(QString::fromStdString(tag));
		this->setItem(row, 0, item1);
		item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
		searchKeys.append(QString::fromStdString(tag));
	}

	PLCParam_ProtocalOpc* opcInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[tag]);
	AtomicVarType valueType = AtomicVarType::None;

	QTableWidgetItem* item2 = nullptr;

	switch (opcInfo->dataType)
	{
	case AtomicVarType::BOOL:
	{
		item2 = new QTableWidgetItem("BOOL");
		valueType = AtomicVarType::BOOL;
		break;
	}
	case AtomicVarType::WORD:
	{
		item2 = new QTableWidgetItem("WORD");
		valueType = AtomicVarType::WORD;
		break;
	}
	case AtomicVarType::DWORD:
	{
		item2 = new QTableWidgetItem("DWORD");
		valueType = AtomicVarType::DWORD;
		break;
	}
	case AtomicVarType::LWORD:
	{
		item2 = new QTableWidgetItem("LWORD");
		valueType = AtomicVarType::LWORD;
		break;
	}
	case AtomicVarType::INT:
	{
		item2 = new QTableWidgetItem("INT");
		valueType = AtomicVarType::INT;
		break;
	}
	case AtomicVarType::DINT:
	{
		item2 = new QTableWidgetItem("DINT");
		valueType = AtomicVarType::DINT;
		break;
	}
	case AtomicVarType::LINT:
	{
		item2 = new QTableWidgetItem("DINT");
		valueType = AtomicVarType::LINT;
		break;
	}
	case AtomicVarType::REAL:
	{
		item2 = new QTableWidgetItem("REAL");
		valueType = AtomicVarType::REAL;
		break;
	}
	case AtomicVarType::LREAL:
	{
		item2 = new QTableWidgetItem("LREAL");
		valueType = AtomicVarType::LREAL;
		break;
	}
	case AtomicVarType::STRING:
	{
		item2 = new QTableWidgetItem("STRING");
		valueType = AtomicVarType::STRING;
		break;
	}
	}
	this->setItem(row, 1, item2);
	item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);
	//address description
	{
		if (opcInfo->protocol == PLCProtocol::OPCUA)
		{
			QString description = QString("%1:%2").arg(opcInfo->ns).arg(QString::fromLocal8Bit(opcInfo->identifier));
			QTableWidgetItem* item3 = new QTableWidgetItem(description);
			this->setItem(row, 2, item3);
			item3->setFlags(item3->flags() & ~Qt::ItemIsEditable);
		}
	}
	//protocol
	{
		QTableWidgetItem* item4 = nullptr;
		switch (opcInfo->protocol)
		{
		case PLCProtocol::OPCUA:
		{
			item4 = new QTableWidgetItem("OPCUA");
			break;
		}
		case PLCProtocol::ModbusTCP:
		{
			item4 = new QTableWidgetItem("ModbusTCP");
			break;
		}
		case PLCProtocol::ModbusRTU:
		{
			item4 = new QTableWidgetItem("ModbusRTU");
			break;
		}
		}
		this->setItem(row, 3, item4);
		item4->setFlags(item4->flags() & ~Qt::ItemIsEditable);
	}
	//current Value
	HmiTemplateTableCellItem* item5 = new HmiTemplateTableCellItem(tag, valueType);
	item5->setFlags(item5->flags() & ~Qt::ItemIsEditable);

	item5->BindParam(tag);
	ScadaScheduler::GetInstance()->AddNode(item5);
	monitorItemMap[tag] = item5;
	this->setItem(row, 4, item5);

	//last Update time
	QDateTime currentTime = QDateTime::currentDateTime();
	QTableWidgetItem* item6 = new QTableWidgetItem(
		currentTime.toString("yyyy-MM-dd HH:mm:ss")
	);
	item6->setFlags(item6->flags() & ~Qt::ItemIsEditable);
	this->setItem(row, 5, item6);
	item6->setBackground(QColor(208, 208, 200, 155));

	item5->timmerCellItem = item6;
}

void HmiTemplateTableMonitor::CreateHeader()
{
	QStringList headers;
	headers << "变量" << "类型" << "地址" << "协议" << "当前值" << "上一次更新时间";
	this->setHorizontalHeaderLabels(headers);
	headerLabels = headers;
}

QTableWidgetItem* HmiTemplateTableMonitor::GetItemAtEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());
	if (index.isValid())
	{
		if (headerLabels[index.column()] == "当前值")
		{
			return item(index.row(), index.column());
		}
	}
	return nullptr;
}
