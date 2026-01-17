#pragma once
#include <QColor>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <UI/ClickableLabel.h>
#include <QListWidget>

namespace CNCSYS
{
	class EntityVGPU;
}

class SectionConfigItems : public QWidget
{
public:
	SectionConfigItems();
	~SectionConfigItems();

public:
	ClickableLabel* colorBlock = nullptr;
	QLineEdit* sectionIdLabel = nullptr;
	QLineEdit* remarkLabel = nullptr;
	QLineEdit* startPosLabel = nullptr;
	QLineEdit* endPosLabel = nullptr;
	QPushButton* btnDelete = nullptr;

	QColor sectionColor = QColor(255,255,255);
};

class SectionConfigPage : public QWidget
{
	friend class SectionConfigItems;

public:
	SectionConfigPage();
	~SectionConfigPage();

	void AddNewItem();

private slots:
	void OnConfirm();

public:
	void BindEntity(CNCSYS::EntityVGPU* entity);

private:
	QListWidget* itemLists = nullptr;
	QPushButton* btnAdd = nullptr;
	QPushButton* btnConfirm = nullptr;

	std::vector<SectionConfigItems*> itemWidgets;
	CNCSYS::EntityVGPU* attachedEntity = nullptr;
};