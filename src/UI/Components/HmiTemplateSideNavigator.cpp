#include "UI/Components/HmiTemplateSideNavigator.h"

#include <QLabel>

NavImageItem::NavImageItem(int row,CanvasGPU* canvas, SketchGPU* sketch, const StationConfig& config) : row(row),config(config),canvas(canvas),sketch(sketch)
{
	this->setFixedSize(config.canvasWidth, config.canvasHeight);
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	displayImg = new QLabel();
	displayImg->show();
	QString formattedNum = QString("%1").arg(row+1,2,10);
	QString htmlText = QString("<u>%1</u>").arg(formattedNum);
	indexLabel = new QLabel(htmlText,this);
	indexLabel->setFixedWidth(20);
	if (canvas != nullptr && sketch != nullptr)
	{
		displayImg->setPixmap(QPixmap::fromImage(canvas->GrabImage(sketch, sketch->attachedOCS, config.canvasWidth,config.canvasHeight,glm::vec4(255.0/255,255.0/255,255.0/255,0.5))));
	}
	layout->addWidget(displayImg);
	QHBoxLayout* hlay = new QHBoxLayout();
	hlay->setContentsMargins(0, 0, 0, 0); // 确保没有内边距干扰
	hlay->setSpacing(0);
	hlay->addStretch(1);
	hlay->addWidget(indexLabel, Qt::AlignCenter);
	hlay->addStretch(1);
	layout->addLayout(hlay);
	this->setLayout(layout);
}

NavImageItem::~NavImageItem()
{

}

void NavImageItem::UpdateImage()
{
	displayImg->setPixmap(QPixmap::fromImage(canvas->GrabImage(sketch, sketch->attachedOCS, config.canvasWidth, config.canvasHeight)));
}

SideNavigator::SideNavigator()
{
	QVBoxLayout* vlayout = new QVBoxLayout();
	NavLists = new QListWidget();
	vlayout->addWidget(NavLists);
	vlayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(vlayout);
}

SideNavigator::~SideNavigator()
{

}

NavImageItem* SideNavigator::AddNavItem(CanvasGPU* canvas, SketchGPU* sketch, const StationConfig& config)
{
	NavImageItem* nav = new NavImageItem(NavLists->count(),canvas, sketch, config);
	QListWidgetItem* itemNew = new QListWidgetItem();
	itemNew->setSizeHint(QSize(config.canvasWidth, config.canvasHeight));
	NavLists->addItem(itemNew);
	navImgs.push_back(nav);
	NavLists->setItemWidget(itemNew,nav);
	return nav;
}