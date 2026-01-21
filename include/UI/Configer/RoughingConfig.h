#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include "Common/ProcessCraft.h"

class RoughingConfigPage : public QWidget
{
public:
	static RoughingConfigPage* GetInstance();

private:
	RoughingConfigPage();
	~RoughingConfigPage();

private:
	static RoughingConfigPage* s_instance;
	static RoughingParamSettings s_setting;
	
	QLabel* labelDirection = nullptr;
	QComboBox* comboDirection = nullptr;
	QLabel* labelTolerance = nullptr;
	QLineEdit* editTolerance = nullptr;
	QLabel* labelRemain = nullptr;
	QLineEdit* editRemain = nullptr;
	QLabel* labelLineSpacing = nullptr;
	QLineEdit* editLineSpacing = nullptr;

	QPushButton* btnExec = nullptr;
	QPushButton* btnComfirm = nullptr;
	QPushButton* btnCancel = nullptr;
};