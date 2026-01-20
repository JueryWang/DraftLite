#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>

class RoughingConfigPage : public QWidget
{
public:
	static RoughingConfigPage* GetInstance();

private:
	RoughingConfigPage();
	~RoughingConfigPage();

private:
	static RoughingConfigPage* s_instance;
	QLabel* labelDirection = nullptr;
	QComboBox* comboDirection = nullptr;
	QLabel* labelTolerance = nullptr;
	QLineEdit* editTolerance = nullptr;
	QLabel* labelRemain = nullptr;
	QLineEdit* editRemain = nullptr;
	QLabel* labelLineSpacing = nullptr;
	QLineEdit* editLineSpacing = nullptr;
};