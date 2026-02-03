#include "UI/Configer/RegionPlannerConfig.h"
#include <QHBoxLayout>
RegionPlannerConfigPage* RegionPlannerConfigPage::s_instance = nullptr;

RegionPlannerConfigPage* RegionPlannerConfigPage::GetInstance()
{
	if (RegionPlannerConfigPage::s_instance == nullptr)
	{
		RegionPlannerConfigPage::s_instance = new RegionPlannerConfigPage();
	}
	return RegionPlannerConfigPage::s_instance;
}

RegionPlannerConfigPage::RegionPlannerConfigPage()
{
	this->setWindowTitle(tr("开粗区域规划配置"));
	this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	preSetPage = new RegionPlannerPreGroupPage();
	preSetPage->show();

	std::vector<RegionParamSettings> regionSettings;
	postSetPage = new RegionPlannerPostGroupPage();

	postSetPage->show();

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addWidget(preSetPage);

	btnTransfer = new QPushButton();
	btnTransfer->setFixedSize(30, 30);
	QIcon iconTransfer("Resources/icon/regionSelect.png");
	btnTransfer->setIcon(iconTransfer);
	btnTransfer->setIconSize(QSize(20, 20));

	connect(btnTransfer, &QPushButton::clicked, this, [&]()
		{
			QListWidgetItem* currentGroup = postSetPage->postSettingGroups->currentItem();
			if (currentGroup)
			{
				std::vector<RegionParamSettings> settingsToTransfer;
				std::vector<RegionPlannerPreItem*> itemToRemove;
				for(int i = 0; i < preSetPage->items.size(); i++)
				{
					RegionPlannerPreItem* preItem = preSetPage->items[i];
					if (preItem->attachedWidget->ifValid->isChecked())
					{
						RegionParamSettings setting;
						setting.regionId = preItem->attachedWidget->regionName->text().toInt();
						setting.dir = (preItem->attachedWidget->rotationDir->currentIndex() == 0) ? GeomDirection::CCW : GeomDirection::CW;
						setting.rotation = preItem->attachedWidget->rotationValue->currentText().toDouble();
						settingsToTransfer.push_back(setting);
						itemToRemove.push_back(preItem);
					}
				}

				for (RegionPlannerPreItem* item : itemToRemove)
				{
					preSetPage->RemoveItem(item);
				}

				RegionPlannerPostItemGroup* groupWidget = postSetPage->groupMap[currentGroup];
				groupWidget->AppendGroups(settingsToTransfer);
			}
		});

	hlayout->addWidget(btnTransfer);
	hlayout->addWidget(postSetPage);

	this->setLayout(hlayout);
}

RegionPlannerConfigPage::~RegionPlannerConfigPage()
{

}
