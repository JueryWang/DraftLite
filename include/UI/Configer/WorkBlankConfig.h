#pragma once
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <map>

namespace CNCSYS
{
	class SketchGPU;
	class EntRingConnection;
	class EntityVGPU;
}

class WorkBlankConfigPage : public QWidget
{
public:
	static WorkBlankConfigPage* GetInstance();
	static void BindRing(CNCSYS::EntRingConnection* ring);
private:
	WorkBlankConfigPage();
	~WorkBlankConfigPage();

public:
	static CNCSYS::EntRingConnection* s_attachedRing;
	static CNCSYS::EntityVGPU* s_workBlank;

private:
	static WorkBlankConfigPage* s_instance;
	std::map<CNCSYS::SketchGPU*, CNCSYS::EntRingConnection*> ringMapper;
	std::map<CNCSYS::EntRingConnection*, CNCSYS::EntityVGPU*> workBlankMapper;
	int regionNum = 0;

	QLabel* labelConfigWidth = nullptr;
	QLabel* labelConfigHeight = nullptr;

	QLineEdit* widthEdit = nullptr;
	QLineEdit* heightEdit = nullptr;

	QPushButton* fitBtn = nullptr;
	QPushButton* confirmBtn = nullptr;
	QPushButton* cancelBtn = nullptr;

};