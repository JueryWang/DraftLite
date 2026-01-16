#pragma once
#include <QColor>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <UI/ClickableLabel.h>

class SectionConfigItems : public QWidget
{
public:
	SectionConfigItems();
	~SectionConfigItems();

public:
	ClickableLabel* colorBlock = nullptr;
	QLineEdit* sectionIdLabel = nullptr;
	QLineEdit* peiceNameLabel = nullptr;
};