#include "UI/MotionControl.h"
#include "Controls//GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "UI/Components/HmiTemplateMsgBox.h"
#include <UI/Components/HmiInterfaceDefines.h>
#include "UI/SizeDefines.h"
#include "UI/GCodeEditor.h"
#include "Common/ProgressInfo.h"
#include "NetWork/FTPClient.h"
#include "Graphics/Anchor.h"
#include <QGridLayout>

#define CHECK_PLC_CONNECTION() \
	if (!g_opcuaClient) \
	{ \
		HmiTemplateMsgBox::warning(this, "错误", "还未连接PLC,无法操作变量",{ "","","确定" }, { nullptr,nullptr,nullptr }); \
		return; \
	}

MotionControl::MotionControl()
{
	QBoxLayout* overallLay = new QVBoxLayout();
	QHBoxLayout* hlayRow1 = new QHBoxLayout();

	this->setFixedSize(MotionControlPanelWith_Ratio * screen_resolution_x, MotionControlPanelHeight_Ratio * screen_resolution_y);
	{
		QWidget* wrapper = new QWidget();

		btnMoveUp = new HmiTemplateButton(ScreenSizeHintX(MarchingPressButtonWidth_Ratio), ScreenSizeHintX(MarchingPressButtonHeight_Ratio));
		btnMoveUp->setText("Y+");
		btnMoveUp->SetBindTag("gvlGlobalData.stCommandCADWork.stManualAxisY.xJogForward");
		connect(btnMoveUp, &QPushButton::pressed, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = true;
			WritePLC_OPCUA(btnMoveUp->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		connect(btnMoveUp, &QPushButton::released, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = false;
			WritePLC_OPCUA(btnMoveUp->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnMoveUp->SetUpdateRate(50);

		btnMoveRight = new HmiTemplateButton(ScreenSizeHintX(MarchingPressButtonWidth_Ratio), ScreenSizeHintX(MarchingPressButtonHeight_Ratio));
		btnMoveRight->setText("X+");
		btnMoveRight->SetBindTag("gvlGlobalData.stCommandCADWork.stManualAxisX.xJogForward");
		connect(btnMoveRight, &QPushButton::pressed, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = true;
			WritePLC_OPCUA(btnMoveRight->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		connect(btnMoveRight, &QPushButton::released, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = false;
			WritePLC_OPCUA(btnMoveRight->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnMoveRight->SetUpdateRate(50);

		btnMoveLeft = new HmiTemplateButton(ScreenSizeHintX(MarchingPressButtonWidth_Ratio), ScreenSizeHintX(MarchingPressButtonHeight_Ratio));
		btnMoveLeft->setText("X-");
		btnMoveLeft->SetBindTag("gvlGlobalData.stCommandCADWork.stManualAxisX.xJogBackward");
		connect(btnMoveLeft, &QPushButton::pressed, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = true;
			WritePLC_OPCUA(btnMoveLeft->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		connect(btnMoveLeft, &QPushButton::released, [this]() {
			PLC_TYPE_BOOL newValue = false;
			WritePLC_OPCUA(btnMoveLeft->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnMoveLeft->SetUpdateRate(50);

		btnMoveDown = new HmiTemplateButton(ScreenSizeHintX(MarchingPressButtonWidth_Ratio), ScreenSizeHintX(MarchingPressButtonHeight_Ratio));
		btnMoveDown->setText("Y-");
		btnMoveDown->SetBindTag("gvlGlobalData.stCommandCADWork.stManualAxisY.xJogBackward");
		connect(btnMoveDown, &QPushButton::pressed, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = true;
			WritePLC_OPCUA(btnMoveDown->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		connect(btnMoveDown, &QPushButton::released, [this]() {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL newValue = false;
			WritePLC_OPCUA(btnMoveDown->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		btnMoveDown->SetUpdateRate(50);

		QHBoxLayout* hlay1 = new QHBoxLayout();
		hlay1->addWidget(btnMoveUp, Qt::AlignCenter);
		QHBoxLayout* hlay2 = new QHBoxLayout();
		hlay2->addWidget(btnMoveLeft,Qt::AlignLeft);
		hlay2->addSpacerItem(new QSpacerItem(100,40,QSizePolicy::Maximum,QSizePolicy::Maximum));
		hlay2->addWidget(btnMoveRight, Qt::AlignRight);
		QHBoxLayout* hlay3 = new QHBoxLayout();
		hlay3->addWidget(btnMoveDown,Qt::AlignCenter);

		QVBoxLayout* vlay = new QVBoxLayout();
		//vlay->addSpacerItem(new QSpacerItem(400, 400, QSizePolicy::Expanding, QSizePolicy::Expanding));
		vlay->addLayout(hlay1);
		vlay->addLayout(hlay2);
		vlay->addLayout(hlay3);
		wrapper->setLayout(vlay);
		hlayRow1->addWidget(wrapper);
	}

	/*QVBoxLayout* vlayCol2 = new QVBoxLayout();
	QHBoxLayout* hlayCol2Row1 = new QHBoxLayout();

	{
		QVBoxLayout* vlay = new QVBoxLayout();
		labelStateAuto = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelStateAuto->SetBindTag("gvlGlobalData.stStatusCADWork.iStateAuto");
		labelStateAuto->SetTextPrefix("自动状态机 ");
		labelStateAuto->SetUpdateRate(30);
		vlay->addWidget(labelStateAuto);

		labelCNCPosX = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelCNCPosX->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordCNC.fPositionAxisX");
		labelCNCPosX->SetTextPrefix("CNC坐标X ");
		labelCNCPosX->SetUpdateRate(30);
		vlay->addWidget(labelCNCPosX);

		labelCNCPosY = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelCNCPosY->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordCNC.fPositionAxisY");
		labelCNCPosY->SetTextPrefix("CNC坐标Y ");
		labelCNCPosY->SetUpdateRate(30);
		vlay->addWidget(labelCNCPosY);

		labelCNCPosZ = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelCNCPosZ->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordCNC.fPositionAxisZ");
		labelCNCPosZ->SetTextPrefix("CNC坐标Z ");
		labelCNCPosZ->SetUpdateRate(30);
		vlay->addWidget(labelCNCPosZ);

		labelAxisPosX = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelAxisPosX->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordAxis.fPositionAxisX");
		labelAxisPosX->SetTextPrefix("轴坐标X ");
		labelAxisPosX->SetUpdateRate(30);
		vlay->addWidget(labelAxisPosX);

		labelAxisPosY = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelAxisPosY->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordAxis.fPositionAxisY");
		labelAxisPosY->SetTextPrefix("轴坐标Y ");
		labelAxisPosY->SetUpdateRate(30);
		vlay->addWidget(labelAxisPosY);

		labelAxisPosZ = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelAxisPosZ->SetBindTag("gvlGlobalData.stStatusCADWork.stCoordAxis.fPositionAxisZ");
		labelAxisPosZ->SetTextPrefix("轴坐标Z ");
		labelAxisPosZ->SetUpdateRate(30);
		vlay->addWidget(labelAxisPosZ);

		labelVelocityCNC = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelVelocityCNC->SetBindTag("gvlGlobalData.stStatusCADWork.fActVelocityCNC");
		labelVelocityCNC->SetTextPrefix("当前给进速度 ");
		labelVelocityCNC->SetUpdateRate(30);
		vlay->addWidget(labelVelocityCNC);

		labelInterpStatusCNC = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelInterpStatusCNC->SetBindTag("gvlGlobalData.stStatusCADWork.nIpoStatusCNC");
		labelInterpStatusCNC->SetTextPrefix("插补状态 ");
		labelInterpStatusCNC->SetUpdateRate(30);
		vlay->addWidget(labelInterpStatusCNC);

		labelCurrentRowCNC = new HmiTemplateLabel(ScreenSizeHintX(MotionNormalLabelWidth), ScreenSizeHintY(MotionNoarmalLabelHeight));
		labelCurrentRowCNC->SetBindTag("gvlGlobalData.stStatusCADWork.iCurrentRowCNC");
		labelCurrentRowCNC->SetTextPrefix("CNC执行行号 ");
		labelCurrentRowCNC->SetUpdateRate(30);
		labelCurrentRowCNC->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_DINT>* var = static_cast<AtomicVar<PLC_TYPE_DINT>*>(param);
			PLC_TYPE_DINT val = var->GetValue();
			QMetaObject::invokeMethod(GCodeEditor::GetInstance(), "SetMarkLine",
				Qt::QueuedConnection,
				Q_ARG(int,val));
		};
		vlay->addWidget(labelCurrentRowCNC);

		hlayCol2Row1->addLayout(vlay);
	}

	{
		QGridLayout* gridLay = new QGridLayout();
		btnSimulate = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnSimulate->setText("模拟");
		btnSimulate->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_AutoPRGTestStart");
		connect(btnSimulate, &QPushButton::pressed, [this]()
		{
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL oldValue;
			ReadPLC_OPCUA(btnSimulate->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
			PLC_TYPE_BOOL newValue = !oldValue;
			WritePLC_OPCUA(btnSimulate->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnSimulate->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				bool success = QMetaObject::invokeMethod(btnSimulate, "SetPressingStyle",
					Qt::QueuedConnection);
			}
			else
			{
				bool success = QMetaObject::invokeMethod(btnSimulate, "SetReleasingStyle",
					Qt::QueuedConnection);
			}
			};
		gridLay->addWidget(btnSimulate,0,0);
		
		btnHandAuto = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnHandAuto->setText("手动");
		btnHandAuto->SetBindTag("gvlGlobalData.stCommandCADWork.xModeAuto");
		connect(btnHandAuto, &QPushButton::pressed, [this]()
		{
			CHECK_PLC_CONNECTION()
			PLC_TYPE_BOOL oldValue;
			ReadPLC_OPCUA(btnHandAuto->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
			PLC_TYPE_BOOL newValue = !oldValue;
			WritePLC_OPCUA(btnHandAuto->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnHandAuto->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				btnHandAuto->setText("自动");
				QMetaObject::invokeMethod(btnHandAuto, "SetPressingStyle",
					Qt::QueuedConnection);
			}
			else
			{
				btnHandAuto->setText("手动");
				QMetaObject::invokeMethod(btnHandAuto, "SetReleasingStyle",
					Qt::QueuedConnection);

			}
			};
		gridLay->addWidget(btnHandAuto,0,1);

		btnDryRun = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnDryRun->setText("空走");
		btnDryRun->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_AutoDryRun");
		connect(btnDryRun, &QPushButton::pressed, [this]()
		{
			CHECK_PLC_CONNECTION()
			Anchor::GetInstance()->animatorOpen = true;
			GCodeEditor::GetInstance()->setReadOnly(true);
			PLC_TYPE_BOOL oldValue;
			ReadPLC_OPCUA(btnDryRun->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
			PLC_TYPE_BOOL newValue = !oldValue;
			WritePLC_OPCUA(btnDryRun->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
		});
		btnDryRun->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				QMetaObject::invokeMethod(btnDryRun, "SetPressingStyle",
					Qt::QueuedConnection);
			}
			else
			{
				QMetaObject::invokeMethod(btnDryRun, "SetReleasingStyle",
					Qt::QueuedConnection);
			}
			};
		gridLay->addWidget(btnDryRun, 1, 0);

		btnDurTest = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnDurTest->setText("耐久测试");
		btnDurTest->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_AutoDurTestStart");
		connect(btnDurTest, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL oldValue;
				ReadPLC_OPCUA(btnDurTest->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
				PLC_TYPE_BOOL newValue = !oldValue;
				WritePLC_OPCUA(btnDurTest->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		btnDurTest->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				QMetaObject::invokeMethod(btnDurTest, "SetPressingStyle",
					Qt::QueuedConnection);
			}
			else
			{
				QMetaObject::invokeMethod(btnDurTest, "SetReleasingStyle",
					Qt::QueuedConnection);
			}
			};
		gridLay->addWidget(btnDurTest, 1, 1);

		btnStart = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnStart->setText("启动");
		btnStart->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_AutoStart");
		connect(btnStart, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				GCodeEditor::GetInstance()->onEditingFinished();
				PLC_TYPE_BOOL oldValue;
				ReadPLC_OPCUA(btnStart->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
				PLC_TYPE_BOOL newValue = !oldValue;
				WritePLC_OPCUA(btnStart->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		btnStart->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				QMetaObject::invokeMethod(btnStart, "SetPressingStyle",
					Qt::QueuedConnection);
			}
			else
			{
				QMetaObject::invokeMethod(btnStart, "SetReleasingStyle",
					Qt::QueuedConnection);
				Anchor::GetInstance()->animatorOpen = true;
			}
			};
		gridLay->addWidget(btnStart, 2, 0);

		btnPause = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnPause->setText("暂停");
		btnPause->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_Pause");
		connect(btnPause, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL oldValue;
				ReadPLC_OPCUA(btnPause->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
				PLC_TYPE_BOOL newValue = !oldValue;
				WritePLC_OPCUA(btnPause->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		gridLay->addWidget(btnPause, 2, 1);

		btnHoming = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnHoming->setText("回原");
		btnHoming->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_AllHoming");
		connect(btnHoming, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = true;
				WritePLC_OPCUA(btnHoming->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		connect(btnHoming, &QPushButton::released, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = false;
				WritePLC_OPCUA(btnHoming->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		gridLay->addWidget(btnHoming, 3, 0);

		btnStop = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnStop->setText("停止");
		btnStop->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_Stop");
		connect(btnStop, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL oldValue = true;
				ReadPLC_OPCUA(btnStop->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
				Anchor::GetInstance()->ResetAnimation();
			});
		connect(btnStop, &QPushButton::released, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL oldValue = false;
				WritePLC_OPCUA(btnStop->bindTag.c_str(), &oldValue, AtomicVarType::BOOL);
			});
		btnStop->valueChangedCallback = [this](void* param) {
			AtomicVar<PLC_TYPE_BOOL>* var = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param);
			PLC_TYPE_BOOL val = var->GetValue();
			if (val)
			{
				Anchor::GetInstance()->ResetAnimation();
			}
		};
		gridLay->addWidget(btnStop, 3, 1);

		btnJogForward = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnJogForward->setText("前进");
		btnJogForward->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_JogForwardCNC");
		connect(btnJogForward, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = true;
				WritePLC_OPCUA(btnJogForward->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		connect(btnJogForward, &QPushButton::released, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = false;
				WritePLC_OPCUA(btnJogForward->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		gridLay->addWidget(btnJogForward, 4, 0);

		btnJogBackward = new HmiTemplateButton(ScreenSizeHintX(MotionNormalButtonWidth_Ratio), ScreenSizeHintY(MotionNormalButtonHeight_Ratio));
		btnJogBackward->setText("后退");
		btnJogBackward->SetBindTag("gvlGlobalData.stCommandCADWork.xHMI_JogBackwardCNC");
		connect(btnJogBackward, &QPushButton::pressed, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = true;
				WritePLC_OPCUA(btnJogBackward->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		connect(btnJogBackward, &QPushButton::released, [this]()
			{
				CHECK_PLC_CONNECTION()
				PLC_TYPE_BOOL newValue = false;
				WritePLC_OPCUA(btnJogBackward->bindTag.c_str(), &newValue, AtomicVarType::BOOL);
			});
		gridLay->addWidget(btnJogBackward, 4, 1);
		hlayCol2Row1->addLayout(gridLay);

	}

	hlayRow1->addLayout(hlayCol2Row1);
	overallLay->addLayout(hlayRow1);
	{
		QVBoxLayout* vlay = new QVBoxLayout();
		vlay->setContentsMargins(10, 0, 10, 0);
		QHBoxLayout* hlay1 = new QHBoxLayout();
		QLabel* descriptionProcessRate = new QLabel("加工倍率");
		descriptionProcessRate->setFixedHeight(40);
		hlay1->addWidget(descriptionProcessRate);
		sldrProcessOverride = new HmiTemplateSlider(300,40);
		sldrProcessOverride->setOrientation(Qt::Horizontal);
		sldrProcessOverride->SetBindTag("gvlGlobalData.stParameterCADWork.fWorkOvrride");
		sldrProcessOverride->setMinimum(50);
		sldrProcessOverride->setMaximum(100);
		sldrProcessOverride->setSingleStep(1);
		connect(sldrProcessOverride, &QSlider::valueChanged, [this](int value){
			CHECK_PLC_CONNECTION()
			PLC_TYPE_LREAL newValue = value;
			WritePLC_OPCUA(sldrProcessOverride->bindTag.c_str(), &newValue, AtomicVarType::LREAL);
		});

		hlay1->addWidget(sldrProcessOverride);
		vlay->addLayout(hlay1);

		QHBoxLayout* hlay2 = new QHBoxLayout();
		QLabel* descriptionSimRate = new QLabel("模拟倍率");
		descriptionSimRate->setFixedHeight(40);
		hlay2->addWidget(descriptionSimRate);
		sldrSimOverride = new HmiTemplateSlider(300, 40);
		sldrSimOverride->setOrientation(Qt::Horizontal);
		sldrSimOverride->SetBindTag("gvlGlobalData.stParameterCADWork.fPRGTeseOvrride");
		sldrSimOverride->setMinimum(50);
		sldrSimOverride->setMaximum(100);
		sldrSimOverride->setSingleStep(1);
		connect(sldrSimOverride, &QSlider::valueChanged, [this](int value) {
			CHECK_PLC_CONNECTION()
			PLC_TYPE_LREAL newValue = value;
			WritePLC_OPCUA(sldrSimOverride->bindTag.c_str(), &newValue, AtomicVarType::LREAL);
		});
		hlay2->addWidget(sldrSimOverride);
		vlay->addLayout(hlay2);

		vlayCol2->addLayout(vlay);
	}
	overallLay->addLayout(vlayCol2);
	overallLay->setSpacing(0);
	overallLay->setContentsMargins(0, 0, 0, 0);*/
	this->setLayout(overallLay);
}

MotionControl::~MotionControl()
{
}
