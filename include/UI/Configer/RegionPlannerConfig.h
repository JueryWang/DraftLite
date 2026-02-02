#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "UI/Configer/RegionPlannerPreItem.h"
#include "UI/Configer/RegionPlannerPostItem.h"

class RegionPlannerConfigPage : public QWidget
{
public:
	static RegionPlannerConfigPage* GetInstance();

private:
	RegionPlannerConfigPage();
	~RegionPlannerConfigPage();

public:
	static RegionPlannerConfigPage* s_instance;

	RegionPlannerPreGroupPage* preSetPage = nullptr;
	RegionPlannerPostGroupPage* postSetPage = nullptr;
	QPushButton* btnTransfer = nullptr;
	
};