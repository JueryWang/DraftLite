#include "UI/Configer/WorkBlankConfig.h"
#include "UI/GCodeEditor.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include "Graphics/Canvas.h"
#include "Algorithm/RoughingAlgo.h"
#include <QGridLayout>

using namespace CNCSYS;

CNCSYS::EntRingConnection* WorkBlankConfigPage::s_attachedRing = nullptr;
WorkBlankConfigPage* WorkBlankConfigPage::s_instance = nullptr;

WorkBlankConfigPage* WorkBlankConfigPage::GetInstance()
{
	if (WorkBlankConfigPage::s_instance == nullptr)
	{
		WorkBlankConfigPage::s_instance = new WorkBlankConfigPage();
	}

	return WorkBlankConfigPage::s_instance;
}

void WorkBlankConfigPage::BindRing(CNCSYS::EntRingConnection* _ring)
{
	s_attachedRing = _ring;
}

WorkBlankConfigPage::WorkBlankConfigPage()
{
	QGridLayout* layout = new QGridLayout();
	
	this->setWindowTitle(tr("毛坯设置"));
	this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	labelConfigWidth = new QLabel();
	labelConfigWidth->setObjectName("CraftConfig");
	labelConfigWidth->setText("宽度");
	layout->addWidget(labelConfigWidth, 0, 0, 1, 1);

	widthEdit = new QLineEdit();
	widthEdit->setObjectName("CraftConfig");
	layout->addWidget(widthEdit,0,1,1,2);

	labelConfigHeight = new QLabel();
	labelConfigHeight->setObjectName("CraftConfig");
	labelConfigHeight->setText("高度");
	layout->addWidget(labelConfigHeight,1,0,1,1);

	heightEdit = new QLineEdit();
	heightEdit->setObjectName("CraftConfig");
	layout->addWidget(heightEdit,1,1,1,2);

	fitBtn = new QPushButton("自适应");
	layout->addWidget(fitBtn,2,0,1,1);

	connect(fitBtn, &QPushButton::clicked, [this]() {
		if (WorkBlankConfigPage::s_attachedRing)
		{
			double width = WorkBlankConfigPage::s_attachedRing->bbox.XRange();
			double height = WorkBlankConfigPage::s_attachedRing->bbox.YRange();

			widthEdit->setText(QString::number(width));
			heightEdit->setText(QString::number(height));
		}
	});

	confirmBtn = new QPushButton("确认");
	layout->addWidget(confirmBtn, 2, 1, 1, 1);
	connect(confirmBtn, &QPushButton::clicked, [this]()
	{
		if (!WorkBlankConfigPage::s_attachedRing->workBlank)
		{
			double width = widthEdit->text().toDouble();
			double height = heightEdit->text().toDouble();
			Polyline2DGPU* rectangle = new Polyline2DGPU();
			std::vector<glm::vec3> polynodes;
			polynodes.push_back({width,height,0.0});
			polynodes.push_back({0.0,height,0.0});
			polynodes.push_back({ 0.0,0.0,0.0 });
			polynodes.push_back({ width,0.0,0.0 });
			polynodes.push_back({ width,height,0.0});
			rectangle->SetParameter(polynodes, false, {0.0,0.0,0.0,0.0});
			rectangle->attribColor = g_workBlankColor;
			rectangle->ResetColor();
			rectangle->selectable = false;
			rectangle->isSelected = true;
			g_canvasInstance->GetSketchShared()->AddEntity(rectangle);
			WorkBlankConfigPage::s_attachedRing->workBlank = rectangle;
			RoughingParamSettings roughingParam;
			roughingParam.direction = MillingDirection::CCW;
			Polyline2DGPU* generatePath = RoughingAlgo::GetRoughingPath(WorkBlankConfigPage::s_attachedRing,rectangle->bbox, roughingParam);
			double toolRadius = 10;
			std::string gcode;
			g_MScontext.toolPos = glm::vec3(width + toolRadius,height + toolRadius,0.0f);
			g_MScontext.wcsAnchor = glm::vec3(0, 0, 0);
			g_MScontext.ncstep = 0;
			
			char buffer[256];
			std::sprintf(buffer,"N%03d G81 X%f Y%f\n",g_MScontext.ncstep,g_MScontext.toolPos.x, g_MScontext.toolPos.y);
			gcode += buffer;
			gcode += generatePath->ToNcInstruction(&g_MScontext,false);
			GCodeEditor::GetInstance()->setText(QString::fromStdString(gcode));
		}
	});


	cancelBtn = new QPushButton("取消");
	layout->addWidget(cancelBtn,2,2,1,1);
	connect(confirmBtn, &QPushButton::clicked, [this]()
	{
		this->close();
	});

	this->setLayout(layout);
}

WorkBlankConfigPage::~WorkBlankConfigPage()
{

}
