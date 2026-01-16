#include "UI/Components/HmiTemplateSectionConfiger.h"
#include <QColorDialog>
#include <QHBoxLayout>

SectionConfigItems::SectionConfigItems()
{
	QHBoxLayout* hlay = new	QHBoxLayout();
	hlay = new QHBoxLayout();
	hlay->setSpacing(5);
	colorBlock = new ClickableLabel();
	colorBlock->setFixedSize(30, 30);
	colorBlock->setStyleSheet("background-color: rgb(255, 0, 0);");

	hlay->addWidget(colorBlock);
	sectionIdLabel = new QLineEdit();
	sectionIdLabel->setDisabled(true);
	sectionIdLabel->setObjectName("CraftConfig");
	sectionIdLabel->setText(QStringLiteral("第一段"));
	hlay->addWidget(sectionIdLabel);
	peiceNameLabel = new QLineEdit();
	peiceNameLabel->setDisabled(true);
	peiceNameLabel->setObjectName("CraftConfig");
	peiceNameLabel->setText(QStringLiteral("测试用例"));
	hlay->addWidget(peiceNameLabel);

	this->setLayout(hlay);
}

SectionConfigItems::~SectionConfigItems()
{

}
