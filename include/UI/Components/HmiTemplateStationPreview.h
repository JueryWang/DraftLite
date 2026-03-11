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
#include "UI/MenuLayerTop.h"
#include "UI/SketchInformation.h"
using namespace CNCSYS;

class PreviewItem : public QWidget
{
public:
	PreviewItem(GLWidget* preview,int id = 0);
	~PreviewItem();
public:
	GLWidget* canvas = nullptr;
	QsciScintilla* editor = nullptr;
	SketchInfoPanel* infoPanel = nullptr;
};

