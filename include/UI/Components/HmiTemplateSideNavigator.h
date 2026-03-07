#pragma once
#include <map>
#include <vector>
#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include "Controls/ScadaNode.h"
#include "UI/GLWidget.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
using namespace CNCSYS;

//通信配置
struct StationConfig
{
public:
	StationConfig() = default;
	std::vector<ScadaNode*> comNodes;
	int canvasWidth = 50;
	int canvasHeight = 50;
};

class SideNavigator;

class NavImageItem : public QWidget
{
public:
	NavImageItem(int row = 0,GLWidget* preview = nullptr, SketchGPU* sketch = nullptr, const StationConfig& config = StationConfig());
	~NavImageItem();
	void UpdateImage();
public:
	QLabel* displayImg;
	QLabel* indexLabel;
	StationConfig config;
	GLWidget* preview = nullptr;
	SketchGPU* sketch = nullptr;
	QListWidget* parent;
	std::string fileSource;
	int row;
};


class SideNavigator : public QWidget
{
public:
	SideNavigator();
	~SideNavigator();
	NavImageItem* AddNavItem(GLWidget* preview = nullptr, SketchGPU* sketch = nullptr,const StationConfig& config = StationConfig());

public:
	std::vector<NavImageItem*> navImgs;
	QListWidget* NavLists = nullptr;
};