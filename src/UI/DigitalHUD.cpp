#pragma once
#include "UI/DigitalHUD.h"
#include "UI/DashBoard.h"
#include "UI/SliderBar.h"
#include <QHBoxLayout>

DigitalHUD::DigitalHUD(int width,int height)
{
	QVBoxLayout* vlay = new QVBoxLayout();
	QHBoxLayout* hlay = new QHBoxLayout();

	board = new DashBoard(0.33 * width,0,100);
	board->setFixedSize(0.7 * width, 0.8 * height);
	slider = new SliderBar(0,100);
	slider->setObjectName("hmiTemplate");
	slider->setFixedSize(0.8 * width, 0.2 * height);
	slider->setRange(0, 100);

	QVBoxLayout* vlay1 = new QVBoxLayout();
	description = new QLabel();
	description->setObjectName("digitalHUDDescription");
	description->setFixedSize(0.4*width,0.3 * height);
	description->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	description->setText("速度\n mm/min");
	description->setWordWrap(true);
	vlay1->addWidget(description,Qt::AlignRight);
	numberArray = new QLabel();
	numberArray->setObjectName("digitalHUDNumber");
	numberArray->setFixedSize(0.5 * width, 0.2 * height);
	numberArray->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	numberArray->setText("000000");
	vlay1->addWidget(numberArray, Qt::AlignRight);

	hlay->addWidget(board);
	hlay->addLayout(vlay1);
	hlay->setContentsMargins(0, 0, 0, 0);
	vlay->addLayout(hlay);
	QHBoxLayout* hlSider = new QHBoxLayout();
	hlSider->addSpacing(0.05 * width);
	hlSider->addWidget(slider, Qt::AlignVCenter);
	vlay->addLayout(hlSider);
	connect(slider, &QSlider::valueChanged, board, [&](int value) {board->SetValue(value); numberArray->setText(QString::asprintf("%06d", value)); });
	
	vlay->setContentsMargins(10, 0, 10,0);
	this->setLayout(vlay);
}

DigitalHUD::~DigitalHUD()
{
	delete slider;
	delete board;
	delete description;
	delete numberArray;
}
