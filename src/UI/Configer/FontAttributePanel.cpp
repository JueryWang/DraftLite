#include "UI/Configer/FontAttributePanel.h"
#include <Graphics/Polyline2D.h>
#include <Graphics/Canvas.h>
#include <Graphics/Sketch.h>
#include <QIcon>
#include <QVBoxLayout>
#include <QLineEdit>
#include <fstream>
using namespace CNCSYS;

FontAttributePanel* FontAttributePanel::s_instance = nullptr;

FontAttributePanel* FontAttributePanel::GetInstance()
{
	if (FontAttributePanel::s_instance == nullptr)
	{
		FontAttributePanel::s_instance = new FontAttributePanel();
	}

	return FontAttributePanel::s_instance;
}

void FontAttributePanel::SetAttachedText(Text* text)
{
	this->textEidt->setText(QString::fromLocal8Bit(text->config.content.c_str()));
	this->Xposition->setText(QString::number(text->bbox.min.x,'f',4));
	this->Yposition->setText(QString::number(text->bbox.min.y,'f',4));
	this->Xdimension->setText(QString::number(text->config.xDimension,'f',4));
	this->Ydimension->setText(QString::number(text->config.yDimension,'f',4));
	this->Fontsapcing->setText(QString::number(text->config.spacing,'f',4));
}

FontAttributePanel::FontAttributePanel()
{
	this->setObjectName("FontAttributePanel");
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setStyleSheet(R"(
		QWidget#FontAttributePanel {
        background-color: #f0f0f0;
        border: 1px solid #d0d0d0;   /* 可选，加上边框更清晰 */
    }
	)");
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(5,5,5,5);
	mainLayout->setSpacing(8);

	QHBoxLayout* titleLayout = new QHBoxLayout();
	QLabel* lbText = new QLabel("文本");
	lbText->setStyleSheet(".QLabel{ background-color: #c7d5e3;}");
	titleLayout->addWidget(lbText);
	closeBtn = new QPushButton();
	closeBtn->setFixedSize(QSize(15,15));
	closeBtn->setIconSize(QSize(15, 15));
	closeBtn->setIcon(QIcon("Resources/icon/close-black.png"));
	titleLayout->addWidget(closeBtn);
	mainLayout->addLayout(titleLayout);

	QGroupBox* posSizeGroup = new QGroupBox(this);
	QGridLayout* gridLayout = new QGridLayout(posSizeGroup);

	gridLayout->addWidget(new QLabel("位置"), 0, 1, Qt::AlignCenter);
	gridLayout->addWidget(new QLabel("尺寸[毫米]"), 0, 2, Qt::AlignCenter);

	gridLayout->addWidget(new QLabel("X"),1,0);
	Xposition = new QLineEdit();
	gridLayout->addWidget(Xposition,1,1);
	Xdimension = new QLineEdit();
	gridLayout->addWidget(Xdimension,1,2);

	gridLayout->addWidget(new QLabel("Y"), 2, 0);
	Yposition = new QLineEdit();
	gridLayout->addWidget(Yposition,2,1);
	Ydimension = new QLineEdit();
	gridLayout->addWidget(Ydimension,2,2);
	mainLayout->addWidget(posSizeGroup);

	applyBtn = new QPushButton("应用");
	QHBoxLayout* layButton = new QHBoxLayout();
	layButton->addWidget(applyBtn);
	mainLayout->addLayout(layButton);

	connect(applyBtn, &QPushButton::clicked, this, [&]()
		{
			if (attachedText)
			{
				attachedText->ClearFonts();
			}

			FontConfig config;
			QString textContent = textEidt->toPlainText();
			fontTypeCombo->currentIndex();
			config.x = 0;
			config.y = 0;
			config.content = textContent.toLocal8Bit().toStdString();
			config.mode = FontMode(fontTypeCombo->currentIndex());
			config.fontpath = "C:/Program Files (x86)/Friendess/CypCutE/Fonts/ISO.SHX";
			config.xDimension = 24;
			config.yDimension = 30;
			config.spacing = 10;

			Text* text = new Text(config);
			g_canvasInstance->GetSketchShared()->AddEntity(text);
			g_canvasInstance->GetSketchShared()->UpdateGCode();
			//std::vector<Polyline2DGPU*> textPaths = ParseSVGPath("C:/Users/Admin/Downloads/YFonts.SHX-master/build/Release/Test.svg");
			//for (Polyline2DGPU* path : textPaths)
			//{
			//	g_canvasInstance->GetSketchShared()->AddEntity(path);
			//	g_canvasInstance->GetSketchShared()->UpdateGCode();
			//}
		});

	QHBoxLayout* fontTypeLayout = new QHBoxLayout();
	fontTypeLayout->addWidget(new QLabel("字体类别"));
	fontTypeCombo = new QComboBox();
	fontTypeCombo->addItem("TureType字体");
	fontTypeCombo->addItem("单线字体");
	fontTypeCombo->addItem("点阵字体");
	fontTypeLayout->addWidget(fontTypeCombo);
	mainLayout->addLayout(fontTypeLayout);

	QComboBox* fontSource = new QComboBox();
	fontSource->addItem("Arial");
	mainLayout->addWidget(fontSource);

	QGridLayout* textPropLayout = new QGridLayout();
	Fontsapcing = new QLineEdit();
	textPropLayout->addWidget(new QLabel("间距"),0,0);
	textPropLayout->addWidget(Fontsapcing,0,1);
	textPropLayout->addWidget(new QLabel("行距"), 1, 0);
	textPropLayout->addWidget(RowSpacing, 1, 1);
	mainLayout->addLayout(textPropLayout);

	mainLayout->addWidget(new QLabel("文本"));
	textEidt = new QTextEdit("TEXT");
	mainLayout->addWidget(textEidt);
	this->setLayout(mainLayout);
}

FontAttributePanel::~FontAttributePanel()
{

}
