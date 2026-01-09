#include "UI/ToolPanel.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "UI/TransformBaseHint.h"
#include "UI/VirtualKeyBoard.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "Common/HistoryCmds.h"
#include <QApplication>
#include <QMenu>
#include <QObject>

namespace CNCSYS
{
	ToolPanel::ToolPanel(std::shared_ptr<SketchGPU> sketch) : mSketch(sketch)
	{
		QToolButton* toolCreateLine = new QToolButton(this);
		toolCreateLine->setFocusPolicy(Qt::NoFocus);
		toolCreateLine->setText("线段");
		toolCreateLine->setFixedSize(QSize(50, 50));
		toolCreateLine->setIcon(QIcon(ICOPATH(Line.png)));
		toolCreateLine->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QObject::connect(toolCreateLine, &QToolButton::clicked, this, &ToolPanel::CreateLine);
		this->addWidget(toolCreateLine);
		
		QToolButton* toolCreatePolyline = new QToolButton(this);
		toolCreatePolyline->setFocusPolicy(Qt::NoFocus);
		toolCreatePolyline->setText("多段线");
		toolCreatePolyline->setFixedSize(QSize(50, 50));
		toolCreatePolyline->setIcon(QIcon(ICOPATH(Polyline.png)));
		toolCreatePolyline->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QObject::connect(toolCreatePolyline, &QToolButton::clicked, this, &ToolPanel::CreatePolyline);
		this->addWidget(toolCreatePolyline);

		QToolButton* toolCreateCircle = new QToolButton(this);
		toolCreateCircle->setFocusPolicy(Qt::NoFocus);
		toolCreateCircle->setFixedSize(50, 50);
		toolCreateCircle->setText("圆");
		toolCreateCircle->setIcon(QIcon(ICOPATH(Circle.png)));
		toolCreateCircle->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QObject::connect(toolCreateCircle, &QToolButton::clicked, this, &ToolPanel::CreateCircle);
		this->addWidget(toolCreateCircle);

		QToolButton* toolCreateArc = new QToolButton(this);
		toolCreateArc->setFocusPolicy(Qt::NoFocus);
		toolCreateArc->setFixedSize(50, 50);
		toolCreateArc->setText("圆弧");
		toolCreateArc->setIcon(QIcon(ICOPATH(Arc.png)));
		toolCreateArc->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QObject::connect(toolCreateArc, &QToolButton::clicked, this, &ToolPanel::CreateArc);
		this->addWidget(toolCreateArc);

		QToolButton* toolCreateRectangle = new QToolButton(this);
		toolCreateRectangle->setFocusPolicy(Qt::NoFocus);
		toolCreateRectangle->setFixedSize(50, 50);
		toolCreateRectangle->setText("矩形");
		toolCreateRectangle->setIcon(QIcon(ICOPATH(Rectangle.png)));
		toolCreateRectangle->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		QObject::connect(toolCreateRectangle, &QToolButton::clicked, this, &ToolPanel::CreateRectangle);
		this->addWidget(toolCreateRectangle);

		QToolButton* toolCreateSpline = new QToolButton(this);
		toolCreateSpline->setFocusPolicy(Qt::NoFocus);
		toolCreateSpline->setFixedSize(50, 50);
		toolCreateSpline->setText("样条");
		toolCreateSpline->setIcon(QIcon(ICOPATH(Spline.png)));
		QObject::connect(toolCreateSpline, &QToolButton::clicked, this, &ToolPanel::CreateSpline);
		toolCreateSpline->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		this->addWidget(toolCreateSpline);

		QToolButton* toolCreateOffset = new QToolButton(this);
		toolCreateOffset->setFocusPolicy(Qt::NoFocus);
		toolCreateOffset->setFixedSize(50, 50);
		toolCreateOffset->setIcon(QIcon(ICOPATH(offset.png)));
		toolCreateOffset->setText("补偿");
		QObject::connect(toolCreateOffset,&QToolButton::clicked,this,&ToolPanel::CreateOffset);
		toolCreateOffset->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		this->addWidget(toolCreateOffset);

		QToolButton* toolCreateTransform = new QToolButton();
		toolCreateTransform->setFocusPolicy(Qt::NoFocus);
		toolCreateTransform->setIcon(QIcon(ICOPATH(Transform.png)));
		toolCreateTransform->setText("变换");
		toolCreateTransform->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toolCreateTransform->setPopupMode(QToolButton::MenuButtonPopup);

		QMenu* dropDownTransformMenu = new QMenu(toolCreateTransform);
		
		QAction* actionMove = new QAction("平移", dropDownTransformMenu);
		QAction* actionZoom = new QAction("交互式缩放", dropDownTransformMenu);
		QAction* actionVerticalFlip = new QAction("垂直镜像", dropDownTransformMenu);
		QAction* actionHorizonFlip = new QAction("水平镜像", dropDownTransformMenu);
		QAction* actionAngleFlip = new QAction("任意角度镜像",dropDownTransformMenu);
		QAction* actionRotate = new QAction("旋转",dropDownTransformMenu);

		connect(actionMove, &QAction::triggered, [this]() {
			mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityMove);
		});
		connect(actionZoom, &QAction::triggered, this, [this]()
		{
			mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityScale);
		});
		//connect(actionVerticalFlip, &QAction::triggered,[this](){
		//	mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityVerticalFlip);
		//});
		//connect(actionHorizonFlip, &QAction::triggered,[this]() {
		//	mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityHorizonFlip);
		//});
		connect(actionAngleFlip, &QAction::triggered, [this]()
		{
			mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityMirror);
		});
		connect(actionRotate, &QAction::triggered, [this] {
			mSketch.get()->GetCanvas()->EnterModal(ModalState::EntityRotate);
		});

		actionZoom->setIcon(QIcon(ICOPATH(Zoom.png)));
		actionMove->setIcon(QIcon(ICOPATH(Move.png)));
		actionVerticalFlip->setIcon(QIcon(ICOPATH(flipVertical.png)));
		actionHorizonFlip->setIcon(QIcon(ICOPATH(flipHorizon.png)));
		actionAngleFlip->setIcon(QIcon(ICOPATH(flipAnyAngle.png)));
		actionRotate->setIcon(QIcon(ICOPATH(Rotation.png)));
		dropDownTransformMenu->addAction(actionMove);
		dropDownTransformMenu->addAction(actionZoom);
		dropDownTransformMenu->addAction(actionVerticalFlip);
		dropDownTransformMenu->addAction(actionHorizonFlip);
		dropDownTransformMenu->addAction(actionAngleFlip);
		dropDownTransformMenu->addAction(actionRotate);

		toolCreateTransform->setMenu(dropDownTransformMenu);
		this->addWidget(toolCreateTransform);

		QToolButton* toolCreateArray = new QToolButton(this);
		toolCreateArray->setFocusPolicy(Qt::NoFocus);
		QMenu* dropDownCreateArrayMenu = new QMenu(toolCreateArray);

		QAction* actionCreateRectArray = new QAction("矩形阵列", dropDownCreateArrayMenu);
		actionCreateRectArray->setIcon(QIcon(ICOPATH(Rect-Array.png)));
		QAction* actionCreateRingArray = new QAction("环形阵列", dropDownCreateArrayMenu);
		actionCreateRingArray->setIcon(QIcon(ICOPATH(Circle-Array.png)));

		dropDownCreateArrayMenu->addAction(actionCreateRectArray);
		dropDownCreateArrayMenu->addAction(actionCreateRingArray);

		RectArrayGenDlg* rectArrayDlg = RectArrayGenDlg::GetInstance();
		connect(rectArrayDlg,&RectArrayGenDlg::ConfirmData,mSketch.get(),&SketchGPU::GenRectArray);

		connect(actionCreateRectArray, &QAction::triggered, [this]() {
			RectArrayGenDlg* dlg = RectArrayGenDlg::GetInstance();
			dlg->open();
		});

		connect(actionCreateRingArray, &QAction::triggered, [this]() {
			RingArrayGenDlg* dlg = RingArrayGenDlg::GetInstance();
			connect(dlg, &RingArrayGenDlg::ConfirmData, mSketch.get(), &SketchGPU::GenRingArray);
			dlg->open();
		});

		dropDownTransformMenu->addAction(actionCreateRectArray);
		dropDownTransformMenu->addAction(actionCreateRingArray);
		toolCreateArray->setIcon(QIcon(ICOPATH(Rect-Array.png)));
		toolCreateArray->setMenu(dropDownCreateArrayMenu);
		toolCreateArray->setText("阵列");
		toolCreateArray->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toolCreateArray->setPopupMode(QToolButton::MenuButtonPopup);
		this->addWidget(toolCreateArray);

		mSketch = sketch;
;	}
	ToolPanel::~ToolPanel()
	{

	}

	void ToolPanel::CreateOffset()
	{
		VirtualKeyBoard* vkb = VirtualKeyBoard::GetInstance();
		vkb->SetConfig("设置偏移值", 5);
		connect(vkb, &VirtualKeyBoard::ConfirmData, this, [&](double value) {
				HistoryRecord rec;
				std::vector<OperationCommand> opCmds;

				//std::vector<EntRingConnection*>compounds =  mSketch.get()->SplitPart(mSketch.get()->GetSelectedEntities());
				//for (EntRingConnection* compound : compounds)
				//{
				//	EntityVGPU* offsetPoly = compound->conponents[0]->Offset(value, 1000);
				//	if (offsetPoly != nullptr)
				//	{
				//		mSketch.get()->AddEntity(offsetPoly);
				//	}
				//	OperationCommand cmd = std::make_pair(offsetPoly, "");
				//	opCmds.push_back(cmd);
				//}

				//std::vector<EntityVGPU*> entities = mSketch.get()->GetEntities();
				//std::vector<EntityVGPU*> offsetedEntitys;
				//for (EntityVGPU* ent : entities)
				//{
				//	EntityVGPU* offsetEnt = nullptr;
				//	if (ent->direction == GeomDirection::CCW)
				//	{
				//		offsetEnt = ent->Offset(value, 1000);
				//	}
				//	else if (ent->direction == GeomDirection::CW)
				//	{
				//		offsetEnt = ent->Offset(-value, 1000);
				//	}

				//	if (offsetEnt != nullptr)
				//	{
				//		offsetedEntitys.push_back(offsetEnt);
				//		OperationCommand cmd = std::make_pair(offsetEnt, "");
				//		opCmds.push_back(cmd);
				//	}
				//}

				//mSketch.get()->ClearEntities();
				//for(EntityVGPU* offseted : offsetedEntitys)
				//{
				//	mSketch.get()->AddEntity(offseted);
				//}

				rec.commands = opCmds;
				HistoryRecorder::GetInstance()->PushRecord(rec);
		});
		vkb->show();
	}
	void ToolPanel::CreateLine()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreateLine);
	}
	void ToolPanel::CreatePolyline()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreatePolyline);
	}
	void ToolPanel::CreateCircle()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreateCircle);
	}
	void ToolPanel::CreateArc()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreateArc);
	}
	void ToolPanel::CreateRectangle()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreateRectangle);
	}
	void ToolPanel::CreateSpline()
	{
		mSketch.get()->GetCanvas()->EnterModal(ModalState::CreateSpline);
	}
}
