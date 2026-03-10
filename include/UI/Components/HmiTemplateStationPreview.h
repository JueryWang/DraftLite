#pragma once

#include "UI/GLWidget.h"
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <Graphics/Canvas.h>
#include <QStandardItemModel>
#include <QPlainTextEdit>
#include <vector>
#include <Qsci/qsciscintilla.h>

using namespace CNCSYS;

class PreviewItem : public QWidget
{
public:
	PreviewItem(GLWidget* preview,int id = 0);
	~PreviewItem();
public:
	QLabel* stationId = nullptr;
	QLabel* fileSource = nullptr;
	QsciScintilla* editor = nullptr;
};

class HmiTemplateStationPreview : public QListView
{
public:
	HmiTemplateStationPreview();
	~HmiTemplateStationPreview();
	void AddPreview(GLWidget* preview);
public:
	QStandardItemModel* model;
	std::vector< GLWidget*> previewlists;
};

