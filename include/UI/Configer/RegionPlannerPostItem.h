#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxlayout>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QColorDialog>
#include <vector>
#include "Common/ProcessCraft.h"
#include "UI/ClickableLabel.h"

class RegionPlannerPostItemGroup;

class RegionPlannerPostItemWidget : public QWidget
{
public:
	RegionPlannerPostItemWidget(const RegionParamSettings& _regionSetting);
	~RegionPlannerPostItemWidget();
	
public:
	QLabel* regionNameHint = nullptr;
	QLabel* regionName = nullptr;
	QComboBox* rotationDir = nullptr;
	QComboBox* rotationValue = nullptr;

	QPushButton* discard = nullptr;
	RegionParamSettings regionSetting;

	RegionPlannerPostItemGroup* groupParent = nullptr;
};

class RegionPlannerPostItem : public QListWidgetItem
{
public:
	RegionPlannerPostItem();
	~RegionPlannerPostItem();

	RegionPlannerPostItemWidget* attachedWidget = nullptr;
};

class RegionPlannerPostItemGroup : public QWidget
{
public:
	RegionPlannerPostItemGroup();
	~RegionPlannerPostItemGroup();
	void AppendGroups(const std::vector<RegionParamSettings>& groupSettings);
	void SetGroups(const std::vector<RegionParamSettings>& groupSettings);
	void RemoveItem(RegionPlannerPostItem* item);
	void SetGroupColor(const QColor& color);
	void Clean();
public:
	QListWidget* postSettingList = nullptr;
	std::vector<RegionPlannerPostItem*> items;

	ClickableLabel* colorBlock = nullptr;
	QGroupBox* groupbox = nullptr;
};

class RegionPlannerPostGroupPage : public QWidget
{
public:
	RegionPlannerPostGroupPage();
	~RegionPlannerPostGroupPage();

	void AddGroupItem(RegionPlannerPostItemGroup* group);
	void Clean();

public:
	QListWidget* postSettingGroups = nullptr;
	QPushButton* addNewGroup = nullptr;
	QPushButton* delGroup = nullptr;

	std::map<QListWidgetItem*, RegionPlannerPostItemGroup*> groupMap;
};