#pragma once
#include <QWidget>
#include <QPushButton>
#include "UI/Components/HmiTemplateLabel.h"
#include "UI/Components/HmiTemplateButton.h"
#include "UI/Components/HmiTemplateSliderBar.h"

class MotionControl : public QWidget
{
public:
	MotionControl();
	~MotionControl();

private:
	HmiTemplateButton* btnMoveLeft = nullptr;
	HmiTemplateButton* btnMoveRight = nullptr;
	HmiTemplateButton* btnMoveUp = nullptr;
	HmiTemplateButton* btnMoveDown = nullptr;
	HmiTemplateLabel* labelStateAuto = nullptr;
	HmiTemplateLabel* labelCNCPosX = nullptr;
	HmiTemplateLabel* labelCNCPosY = nullptr;
	HmiTemplateLabel* labelCNCPosZ = nullptr;
	HmiTemplateLabel* labelAxisPosX = nullptr;
	HmiTemplateLabel* labelAxisPosY = nullptr;
	HmiTemplateLabel* labelAxisPosZ = nullptr;
	HmiTemplateLabel* labelVelocityCNC = nullptr;
	HmiTemplateLabel* labelInterpStatusCNC = nullptr;
	HmiTemplateLabel* labelCurrentRowCNC = nullptr;
	HmiTemplateButton* btnSimulate = nullptr;
	HmiTemplateButton* btnHandAuto = nullptr;
	HmiTemplateButton* btnDryRun = nullptr;
	HmiTemplateButton* btnDurTest = nullptr;
	HmiTemplateButton* btnStart = nullptr;
	HmiTemplateButton* btnPause = nullptr;
	HmiTemplateButton* btnHoming = nullptr;
	HmiTemplateButton* btnStop = nullptr;
	HmiTemplateButton* btnJogForward = nullptr;
	HmiTemplateButton* btnJogBackward = nullptr;
	HmiTemplateSlider* sldrProcessOverride = nullptr;
	HmiTemplateSlider* sldrSimOverride = nullptr;
};
