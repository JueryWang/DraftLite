#include "UI/Configer/RegionPlannerPreItem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

RegionPlannerPreItemWidget::RegionPlannerPreItemWidget(int _regionId) : regionId(_regionId)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	regionNameHint = new QLabel("区域编号:");
	regionNameHint->setObjectName("CraftConfig");
	hlay->addWidget(regionNameHint);
	regionName = new QLabel(QString::number(regionId));
	regionName->setObjectName("CraftConfig");
	hlay->addWidget(regionName);
	hlay->addSpacing(20);
	rotationHint = new QLabel("旋转角度");
	rotationHint->setObjectName("CraftConfig");
	hlay->addWidget(rotationHint);
	rotationDir = new QComboBox();
	rotationDir->setObjectName("CraftConfig");
	rotationDir->addItems({ "逆时针","顺时针" });
	hlay->addWidget(rotationDir);
	rotationValue = new QComboBox();
	rotationValue->setObjectName("CraftConfig");
	rotationValue->addItems({ "0","90","180","270" });
	hlay->addWidget(rotationValue);
	hlay->addSpacing(10);
	ifValid = new QCheckBox();
	ifValid->setObjectName("CraftConfig");
	hlay->addWidget(ifValid);

	this->setLayout(hlay);
}

RegionPlannerPreItemWidget::~RegionPlannerPreItemWidget()
{
	delete regionNameHint;
	delete regionName;
	delete rotationHint;
	delete rotationDir;
	delete rotationValue;
	delete ifValid;
}

RegionPlannerPreItem::RegionPlannerPreItem()
{
}

RegionPlannerPreItem::~RegionPlannerPreItem()
{
}

RegionPlannerPreGroupPage::RegionPlannerPreGroupPage()
{
	QVBoxLayout* vlay = new QVBoxLayout();
	preSettingList = new QListWidget();
	vlay->addWidget(preSettingList);
	this->setLayout(vlay);
}

RegionPlannerPreGroupPage::~RegionPlannerPreGroupPage()
{
	for(int i = 0; i < items.size(); i++)
	{
		RegionPlannerPreItem* item = items[i];
		preSettingList->removeItemWidget(item);
		delete item->attachedWidget;
		delete items[i];
	}
	
	items.clear();
}

void RegionPlannerPreGroupPage::SetRegionCount(int regionCount)
{
	Clean();
	for (int i = 0; i < regionCount; i++)
	{
		RegionPlannerPreItemWidget* itemWidget = new RegionPlannerPreItemWidget(i);
		RegionPlannerPreItem* listItem = new RegionPlannerPreItem();
		listItem->attachedWidget = itemWidget;
		QSize itemSize = itemWidget->sizeHint();
		items.push_back(listItem);
		preSettingList->addItem(listItem);
		preSettingList->setItemWidget(listItem, listItem->attachedWidget);
		listItem->setSizeHint(itemSize);
		this->resize(QSize(itemSize.width() + 50, preSettingList->count() * itemSize.height()));
	}
}

void RegionPlannerPreGroupPage::AddItem(RegionParamSettings setting)
{
	RegionPlannerPreItemWidget* itemWidget = new RegionPlannerPreItemWidget(setting.regionId);
	itemWidget->rotationDir->setCurrentIndex((setting.dir == GeomDirection::CCW) ? 0 : 1);
	itemWidget->rotationValue->setCurrentIndex(static_cast<int>(setting.rotation) / 90);
	RegionPlannerPreItem* listItem = new RegionPlannerPreItem();
	listItem->attachedWidget = itemWidget;
	QSize itemSize = itemWidget->sizeHint();
	bool inserted = false;
	for (int i = 0; i < preSettingList->count(); i++)
	{
		RegionPlannerPreItem* targetItem = items[i];
		if (targetItem->attachedWidget->regionId > setting.regionId)
		{
			preSettingList->insertItem(i, listItem);
			preSettingList->setItemWidget(listItem, listItem->attachedWidget);
			listItem->setSizeHint(itemSize);
			inserted = true;
			break;
		}
	}
	if (!inserted)
	{
		preSettingList->insertItem(preSettingList->count(), listItem);
		preSettingList->setItemWidget(listItem, listItem->attachedWidget);
		listItem->setSizeHint(itemSize);
	}
	preSettingList->setItemWidget(listItem, listItem->attachedWidget);
	items.push_back(listItem);
	std::sort(items.begin(), items.end(), [](RegionPlannerPreItem* item1, RegionPlannerPreItem* item2)
		{
			return item1->attachedWidget->regionId < item2->attachedWidget->regionId;
		});

	listItem->setSizeHint(itemSize);
}

void RegionPlannerPreGroupPage::RemoveItem(RegionPlannerPreItem* item)
{
	auto find = std::find(items.begin(), items.end(), item);
	if(find != items.end())
	{
		int index = std::distance(items.begin(), find);
		RegionPlannerPreItem* listItem = items[index];
		preSettingList->removeItemWidget(listItem);
		delete listItem->attachedWidget;
		delete listItem;
		items.erase(find);
	}
}

void RegionPlannerPreGroupPage::Clean()
{
	for (int i = 0; i < items.size(); i++)
	{
		RegionPlannerPreItem* item = items[i];
		preSettingList->removeItemWidget(item);
		delete item->attachedWidget;
		delete items[i];
	}
	preSettingList->clear();

	items.clear();
}