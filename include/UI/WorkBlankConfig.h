#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace CNCSYS
{
	class EntRingConnection;
}

class WorkBlankConfigPage : public QWidget
{
public:
	static WorkBlankConfigPage* GetInstance();
	static void BindRing(CNCSYS::EntRingConnection* ring);

private:
	WorkBlankConfigPage();
	~WorkBlankConfigPage();

private:
	static WorkBlankConfigPage* s_instance;
	static CNCSYS::EntRingConnection* s_attachedRing;

	QLabel* labelConfigWidth = nullptr;
	QLabel* labelConfigHeight = nullptr;

	QLineEdit* widthEdit = nullptr;
	QLineEdit* heightEdit = nullptr;

	QPushButton* fitBtn = nullptr;
	QPushButton* confirmBtn = nullptr;
	QPushButton* cancelBtn = nullptr;

};