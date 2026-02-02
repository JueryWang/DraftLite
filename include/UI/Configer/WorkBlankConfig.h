#pragma once
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>

namespace CNCSYS
{
	class EntRingConnection;
}

class WorkBlankConfigPage : public QWidget
{
public:
	static WorkBlankConfigPage* GetInstance();
	static void BindRing(CNCSYS::EntRingConnection* ring);
	static void SetRegionNum(int Num);
private:
	WorkBlankConfigPage();
	~WorkBlankConfigPage();

public:
	static CNCSYS::EntRingConnection* s_attachedRing;

private:
	static WorkBlankConfigPage* s_instance;
	int regionNum = 0;

	QLabel* labelConfigWidth = nullptr;
	QLabel* labelConfigHeight = nullptr;

	QLineEdit* widthEdit = nullptr;
	QLineEdit* heightEdit = nullptr;

	QPushButton* fitBtn = nullptr;
	QPushButton* confirmBtn = nullptr;
	QPushButton* cancelBtn = nullptr;

};