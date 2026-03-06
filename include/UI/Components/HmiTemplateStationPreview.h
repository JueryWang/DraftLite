#pragma once

#include "UI/GLWidget.h"
#include <QListView>
#include <QGridLayout>
#include <Graphics/Canvas.h>
#include <QStandardItemModel>
#include <vector>
using namespace CNCSYS;

class HmiTemplateStationPreview : QListView
{
public:
	HmiTemplateStationPreview();
	~HmiTemplateStationPreview();
	void AddPreview(GLWidget* preview);
public:
	QStandardItemModel* model;
	std::vector< GLWidget*> previewlists;
};

