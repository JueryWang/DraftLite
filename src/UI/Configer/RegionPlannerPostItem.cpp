#include "UI/Configer/RegionPlannerPostItem.h"
#include "UI/Configer/RegionPlannerConfig.h"
#include "Algorithm/RoughingAlgo.h"
#include <QHBoxLayout>
#include <QIcon>

RegionPlannerPostItemWidget::RegionPlannerPostItemWidget(const RegionParamSettings& _regionSetting) : regionSetting(_regionSetting)
{
	QHBoxLayout* hlay = new QHBoxLayout();

	regionNameHint = new QLabel("区域编号:");
	regionNameHint->setObjectName("CraftConfig");
	hlay->addWidget(regionNameHint);
	regionName = new QLabel(QString::number(_regionSetting.regionId));
	regionName->setObjectName("CraftConfig");
	hlay->addWidget(regionName);
	rotationDir = new QComboBox();
	rotationDir->setObjectName("CraftConfig");
	rotationDir->addItems({ "逆时针","顺时针"});
	switch (regionSetting.dir)
	{
		case GeomDirection::CW:
		{
			rotationDir->setCurrentIndex(1);
			break;
		}
		case GeomDirection::CCW:
		{
			rotationDir->setCurrentIndex(0);
			break;
		}
	}
	rotationDir->setEnabled(false);
	hlay->addWidget(rotationDir);

	rotationValue = new QComboBox();
	rotationValue->setObjectName("CraftConfig");
	rotationValue->addItems({ "0","90","180","270" });
	switch (static_cast<int>(regionSetting.rotation))
	{
		case 0:
		{
			rotationValue->setCurrentIndex(0);
			break;
		}
		case 90:
		{
			rotationValue->setCurrentIndex(1);
			break;
		}
		case 180:
		{
			rotationValue->setCurrentIndex(2);
			break;
		}
		case 270:
		{
			rotationValue->setCurrentIndex(3);
			break;
		}
	}
	rotationValue->setEnabled(false);
	hlay->addWidget(rotationValue);
	hlay->addSpacing(10);
	
	discard = new QPushButton();
	discard->setFixedSize(20,20);
	QIcon btnIcon("Resources/icon/regionDiscard.png");
	discard->setIcon(btnIcon);
	discard->setIconSize(QSize(20, 20));
	connect(discard, &QPushButton::clicked, this, [&]() {
		if (groupParent)
		{
			for (RegionPlannerPostItem* item : groupParent->items)
			{
				if(item->attachedWidget == this)
				{
					groupParent->RemoveItem(item);
					break;
				}
			}
		}
	});
	hlay->addWidget(discard, Qt::AlignVCenter);
	this->setLayout(hlay);
}

RegionPlannerPostItemWidget::~RegionPlannerPostItemWidget()
{
	delete regionNameHint;
	delete regionName;
	delete rotationDir;
	delete rotationValue;
	delete discard;
}

RegionPlannerPostItem::RegionPlannerPostItem()
{
}

RegionPlannerPostItem::~RegionPlannerPostItem()
{

}

RegionPlannerPostItemGroup::RegionPlannerPostItemGroup()
{
	QHBoxLayout* hlay = new QHBoxLayout();
	colorBlock = new ClickableLabel();
	colorBlock->setStyleSheet(R"(
			QLabel{
				background-color: green;
			}
	)");
	colorBlock->setFixedSize(30, 30);
	hlay->addWidget(colorBlock,Qt::AlignTop);
	QVBoxLayout* vlay = new QVBoxLayout();
	postSettingList = new QListWidget();
	vlay->addWidget(postSettingList);
	groupbox = new QGroupBox("区域组");
	groupbox->setLayout(vlay);
	hlay->addWidget(groupbox);
	this->setLayout(hlay);

	connect(colorBlock, &ClickableLabel::clicked, this, [&]()
		{
			QColor selectedColor = QColorDialog::getColor(
				Qt::white,
				this,
				"选择组颜色",
				QColorDialog::ShowAlphaChannel
			);
			this->SetGroupColor(selectedColor);
		});
}

RegionPlannerPostItemGroup::~RegionPlannerPostItemGroup()
{
	Clean();
}

void RegionPlannerPostItemGroup::AppendGroups(const std::vector<RegionParamSettings>& groupSettings)
{
	for (const RegionParamSettings& setting : groupSettings)
	{
		RegionPlannerPostItemWidget* itemWidget = new RegionPlannerPostItemWidget(setting);
		itemWidget->groupParent = this;
		RegionPlannerPostItem* listItem = new RegionPlannerPostItem();
		listItem->setSizeHint(itemWidget->sizeHint());
		listItem->attachedWidget = itemWidget;
		items.push_back(listItem);
		listItem->setSizeHint(listItem->attachedWidget->sizeHint());
		postSettingList->addItem(listItem);
		postSettingList->setItemWidget(listItem, listItem->attachedWidget);
	}
}

void RegionPlannerPostItemGroup::SetGroups(const std::vector<RegionParamSettings>& groupSettings)
{
	Clean();
	for (const RegionParamSettings& setting : groupSettings)
	{
		RegionPlannerPostItemWidget* itemWidget = new RegionPlannerPostItemWidget(setting);
		itemWidget->groupParent = this;
		RegionPlannerPostItem* listItem = new RegionPlannerPostItem();
		listItem->setSizeHint(itemWidget->sizeHint());
		listItem->attachedWidget = itemWidget;
		items.push_back(listItem);
		listItem->setSizeHint(listItem->attachedWidget->sizeHint());
		postSettingList->addItem(listItem);
		postSettingList->setItemWidget(listItem, listItem->attachedWidget);
	}
}

void RegionPlannerPostItemGroup::RemoveItem(RegionPlannerPostItem* item)
{
	RegionPlannerConfigPage* regionConfig = RegionPlannerConfigPage::GetInstance();
	RegionPlannerPostItemWidget* itemWidget = item->attachedWidget;
	RegionPlannerPreItemWidget* preItemWidget = new RegionPlannerPreItemWidget(itemWidget->regionSetting.regionId);
	preItemWidget->rotationDir->setCurrentIndex((itemWidget->regionSetting.dir == GeomDirection::CCW) ? 0 : 1);
	preItemWidget->rotationValue->setCurrentIndex(static_cast<int>(itemWidget->regionSetting.rotation) / 90);
	regionConfig->preSetPage->AddItem(itemWidget->regionSetting);
	
	auto find = std::find(items.begin(), items.end(), item);
	if (find != items.end())
	{
		postSettingList->removeItemWidget(item);
		delete item->attachedWidget;
		delete item;
		items.erase(find);
	}
}

void RegionPlannerPostItemGroup::SetGroupColor(const QColor& color)
{
	colorBlock->setStyleSheet(QString(R"(
		QLabel{
			background-color: rgb(%1,%2,%3);
		}
	)").arg(color.red()).arg(color.green()).arg(color.blue()));

	RegionPlannerPostGroupPage* pageParent = RegionPlannerConfigPage::GetInstance()->postSetPage;
	std::set<int> regionIds;
	for (RegionPlannerPostItem* item : items)
	{
		regionIds.insert(item->attachedWidget->regionName->text().toInt());
	}
	std::map<int, std::vector<PointClusterNode>>&& clusterMap = RoughingAlgo::GetRegionResult();
	for(int regionId : regionIds)
	{
		for (PointClusterNode& node : clusterMap[regionId])
		{
			node.entityParent->attribColor = glm::vec4(color.redF(), color.greenF(), color.blueF(),1.0f);
			node.entityParent->ResetColor();
		}
	}
}

void RegionPlannerPostItemGroup::Clean()
{
	std::vector<RegionParamSettings> removedSettings;

	while(items.size())
	{
		postSettingList->removeItemWidget(items[0]);
 		RemoveItem(items[0]);
	}
	postSettingList->clear();
	items.clear();
}

RegionPlannerPostGroupPage::RegionPlannerPostGroupPage()
{
	QVBoxLayout* vlay = new QVBoxLayout();
	postSettingGroups = new QListWidget();
	vlay->addWidget(postSettingGroups);

	QHBoxLayout* hlayBtn = new QHBoxLayout();
	QWidget* placeHolder = new QWidget();
	placeHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	addNewGroup = new QPushButton("添加组");
	addNewGroup->setObjectName("CraftConfig");
	hlayBtn->addWidget(addNewGroup);
	connect(addNewGroup, &QPushButton::clicked, this, [&]() {
		RegionPlannerPostItemGroup* newGroup = new RegionPlannerPostItemGroup();
		this->AddGroupItem(newGroup);
	});

	delGroup = new QPushButton("删除组");
	delGroup->setObjectName("CraftConfig");
	connect(delGroup, &QPushButton::clicked, this, [&]() {
		QListWidgetItem* currentItem = RegionPlannerConfigPage::GetInstance()->postSetPage->postSettingGroups->currentItem();
		if (currentItem)
		{
			RegionPlannerPostItemGroup* selectedGroup = static_cast<RegionPlannerPostItemGroup*>(RegionPlannerConfigPage::GetInstance()->postSetPage->postSettingGroups->itemWidget(currentItem));
			RegionPlannerConfigPage::GetInstance()->postSetPage->postSettingGroups->removeItemWidget(currentItem);
			delete currentItem;
			delete selectedGroup;
		}
	});
	hlayBtn->addWidget(delGroup);

	vlay->addLayout(hlayBtn);

	this->setLayout(vlay);
}

RegionPlannerPostGroupPage::~RegionPlannerPostGroupPage()
{
	Clean();
}

void RegionPlannerPostGroupPage::Clean()
{
	for(auto& pair : groupMap)
	{
		QListWidgetItem* item = pair.first;
		RegionPlannerPostItemGroup* group = pair.second;
		postSettingGroups->removeItemWidget(item);
		delete group;
		delete item;
	}

	postSettingGroups->clear();
	groupMap.clear();
}

void RegionPlannerPostGroupPage::AddGroupItem(RegionPlannerPostItemGroup* group)
{
	QListWidgetItem* item = new QListWidgetItem();
	postSettingGroups->addItem(item);
	QSize itemSize = group->sizeHint();
	postSettingGroups->setItemWidget(item, group);
	item->setSizeHint(itemSize);
	groupMap[item] = group;
}
