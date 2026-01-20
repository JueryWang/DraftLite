#include "UI/Configer/RoughingConfig.h"
#include <QGridLayout>

RoughingConfigPage* RoughingConfigPage::s_instance = nullptr;

RoughingConfigPage* RoughingConfigPage::GetInstance()
{
	if (RoughingConfigPage::s_instance == nullptr)
	{
		RoughingConfigPage::s_instance = new RoughingConfigPage();
	}
	return RoughingConfigPage::s_instance;
}

RoughingConfigPage::RoughingConfigPage()
{
	QGridLayout* layout = new QGridLayout();

	this->setWindowTitle("开粗配置:");
	this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	labelDirection = new QLabel("切削方向:");
	labelDirection->setMinimumWidth(30);
	labelDirection->setObjectName("CraftConfig");
	layout->addWidget(labelDirection,0,0,1,1);
	
	comboDirection = new QComboBox();
	comboDirection->setObjectName("CraftConfig");
	QStringList directions = {"顺铣","逆铣","任意"};
	comboDirection->setMinimumWidth(120);
	comboDirection->addItems(directions);
	layout->addWidget(comboDirection,0,1,1,2,Qt::AlignRight);

	labelRemain = new QLabel("余量:");
	labelRemain->setMinimumWidth(30);
	labelRemain->setObjectName("CraftConfig");
	layout->addWidget(labelRemain,1,0,1,1);

	editRemain = new QLineEdit();
	editRemain->setObjectName("CraftConfig");
	layout->addWidget(editRemain,1,1,1,1);

	labelLineSpacing = new QLabel("行距:");
	labelLineSpacing->setMinimumWidth(30);
	labelLineSpacing->setObjectName("CraftConfig");
	layout->addWidget(labelLineSpacing,1,2,1,1);

	editLineSpacing = new QLineEdit();
	editLineSpacing->setObjectName("CraftConfig");
	layout->addWidget(editLineSpacing, 1, 3, 1, 1);

	labelTolerance = new QLabel("公差:");
	labelTolerance->setMinimumWidth(30);
	labelTolerance->setObjectName("CraftConfig");
	layout->addWidget(labelTolerance,2,0,1,1);

	editTolerance = new QLineEdit();
	editTolerance->setObjectName("CraftConfig");
	layout->addWidget(editTolerance,2,1,1,1);

	this->setLayout(layout);
}

RoughingConfigPage::~RoughingConfigPage()
{

}
