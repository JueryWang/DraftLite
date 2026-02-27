#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include "UI/SizeDefines.h"

class AboutVersion : public QWidget
{
public:
	AboutVersion();
	~AboutVersion();

private:
	QLabel* poster{nullptr};
	QLabel* textDescription{nullptr};
	QScrollArea* textArea{nullptr};
};