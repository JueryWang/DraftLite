#include "UI/Components/HmiTemplateProgressBar.h"
#include <QPainter>
#include <QStyleOptionProgressBar>
#include <QStyle>

HmiTemplateProgressBar::HmiTemplateProgressBar(int width, int height, int fontsize)
{
	this->setObjectName("hmiTemplate");
	QFont font("NirmalaUI");
	font.setPixelSize(fontsize);
	this->setFont(font);
	this->setFixedSize(width, height);
}

HmiTemplateProgressBar::~HmiTemplateProgressBar()
{

}

void HmiTemplateProgressBar::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::TextAntialiasing);

	QStyleOptionProgressBar option;
	initStyleOption(&option);
	style()->drawControl(QStyle::CE_ProgressBar, &option, &painter, this);
}
