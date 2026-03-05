#include "UI/Components/HmiTemplateSideNavigator.h"

#include <QLabel>

NavImage::NavImage(CanvasGPU* canvas, SketchGPU* sketch, const StationConfig& config) : config(config),canvas()
{
	this->setFixedSize(config.canvasWidth, config.canvasHeight);
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	displayImg = new QLabel();
	displayImg->show();
	if (canvas != nullptr && sketch != nullptr)
	{
		displayImg->setPixmap(QPixmap::fromImage(canvas->GrabImage(sketch, sketch->attachedOCS, config.canvasWidth,config.canvasHeight,glm::vec4(187.0/255,193.0/255,217.0/255,0.5))));
	}
	layout->addWidget(displayImg);
	this->setLayout(layout);
}

NavImage::~NavImage()
{

}

void NavImage::UpdateImage()
{
	displayImg->setPixmap(QPixmap::fromImage(canvas->GrabImage(sketch, sketch->attachedOCS, config.canvasWidth, config.canvasHeight)));
}

SideNavigator::SideNavigator()
{
	vlayout = new QVBoxLayout();
	vlayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(vlayout);
}

SideNavigator::~SideNavigator()
{

}

NavImage* SideNavigator::AddNavItem(CanvasGPU* canvas, SketchGPU* sketch, const StationConfig& config)
{
	NavImage* nav = new NavImage(canvas, sketch, config);
	vlayout->addWidget(nav);
	navImgs.push_back(nav);
	return nav;
}