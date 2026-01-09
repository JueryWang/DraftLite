#pragma once

#include <QProgressBar>

class HmiTemplateProgressBar : public QProgressBar
{
public:
	HmiTemplateProgressBar(int width,int height,int fontsize = 16);
	~HmiTemplateProgressBar();

protected:
	void paintEvent(QPaintEvent* event) override;
};