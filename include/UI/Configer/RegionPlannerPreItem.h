#pragma once
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <vector>
#include "Common/ProcessCraft.h"

class RegionPlannerPreItemWidget : public QWidget
{
public:
	RegionPlannerPreItemWidget(int regionId);
	~RegionPlannerPreItemWidget();

public:
	QLabel* regionNameHint = nullptr;
	QLabel* regionName = nullptr;
	QLabel* rotationHint = nullptr;
	QComboBox* rotationDir = nullptr;
	QComboBox* rotationValue = nullptr;
	QCheckBox* ifValid = nullptr;

	int regionId = -1;
};

class RegionPlannerPreItem : public QListWidgetItem
{
public:
	RegionPlannerPreItem();
	~RegionPlannerPreItem();

	RegionPlannerPreItemWidget* attachedWidget = nullptr;
};

class RegionPlannerPreGroupPage : public QWidget
{
public:
	RegionPlannerPreGroupPage();
	~RegionPlannerPreGroupPage();
	void SetRegionCount(int regionCount);
	void AddItem(RegionParamSettings setting);
	void RemoveItem(RegionPlannerPreItem* item);
	void Clean();

public:
	QListWidget* preSettingList = nullptr;
	std::vector<RegionPlannerPreItem*> items;
};
