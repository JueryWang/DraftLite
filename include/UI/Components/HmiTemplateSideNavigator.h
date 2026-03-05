#pragma once
#include <map>
#include <vector>
#include <QWidget>
#include <QImage>
#include <QVBoxLayout>
#include "Controls/ScadaNode.h"
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

class NavImage : public QWidget
{
public:
	NavImage(CanvasGPU* canvas = nullptr, SketchGPU* sketch = nullptr, const StationConfig& config = StationConfig());
	~NavImage();
	void UpdateImage();
public:
	QLabel* displayImg;
	StationConfig config;
	CanvasGPU* canvas = nullptr;
	SketchGPU* sketch = nullptr;
};


class SideNavigator : public QWidget
{
public:
	SideNavigator();
	~SideNavigator();
	NavImage* AddNavItem(CanvasGPU* canvas = nullptr, SketchGPU* sketch = nullptr,const StationConfig& config = StationConfig());

public:
	std::vector<NavImage*> navImgs;
	QVBoxLayout* vlayout;
};