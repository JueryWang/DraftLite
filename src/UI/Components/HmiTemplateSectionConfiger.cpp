#include "UI/Components/HmiTemplateSectionConfiger.h"
#include <QColorDialog>
#include <QHBoxLayout>

SectionConfigItems::SectionConfigItems()
{
	this->setWindowTitle(tr("选段配置"));
	this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	QHBoxLayout* hlay = new	QHBoxLayout();
	hlay = new QHBoxLayout();
	hlay->setSpacing(5);
	colorBlock = new ClickableLabel();
	colorBlock->setFixedSize(30, 30);
	colorBlock->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(sectionColor.red()).arg(sectionColor.green()).arg(sectionColor.blue()));
	hlay->addWidget(colorBlock);

	connect(colorBlock, &ClickableLabel::clicked, [&]() {
		sectionColor = QColorDialog::getColor(
			sectionColor,
			this,
			tr("选择颜色")
		);
		colorBlock->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(sectionColor.red()).arg(sectionColor.green()).arg(sectionColor.blue()));
	});

	sectionIdLabel = new QLineEdit();
	sectionIdLabel->setDisabled(true);
	sectionIdLabel->setObjectName("CraftConfig");
	sectionIdLabel->setText(QStringLiteral("第一段"));
	hlay->addWidget(sectionIdLabel);

	startPosLabel = new QLineEdit();
	startPosLabel->setDisabled(true);
	startPosLabel->setObjectName("CraftConfig");
	startPosLabel->setText("");
	hlay->addWidget(startPosLabel);;

	endPosLabel = new QLineEdit();
	endPosLabel->setDisabled(true);
	endPosLabel->setObjectName("CraftConfig");
	endPosLabel->setText("");
	hlay->addWidget(endPosLabel);

	remarkLabel = new QLineEdit();
	remarkLabel->setObjectName("CraftConfig");
	remarkLabel->setPlaceholderText(tr("备注.."));
	hlay->addWidget(remarkLabel);

	btnDelete = new QPushButton(tr("删除"));
	btnDelete->setFixedSize(40,25);
	btnDelete->setObjectName("CraftConfig");
	hlay->addWidget(btnDelete);

	this->setLayout(hlay);
}

SectionConfigItems::~SectionConfigItems()
{

}

SectionConfigPage::SectionConfigPage()
{

}

SectionConfigPage::~SectionConfigPage()
{
}

void SectionConfigPage::OnConfirm()
{

}

void SectionConfigPage::AddNewItem()
{
	QListWidgetItem* itemNew = new QListWidgetItem();
	itemNew->setSizeHint(QSize(this->width(),50));
}

void SectionConfigPage::BindEntity(CNCSYS::EntityVGPU* entity)
{
	attachedEntity = entity;
}
