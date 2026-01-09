#pragma once

#include "Controls/ScadaNode.h"
#include <QSlider>

class HmiTemplateSlider : public ScadaNode, public QSlider
{
	friend class ScadaSceduler;
public:
	HmiTemplateSlider(int width, int height);
	~HmiTemplateSlider();
	virtual void UpdateNode() override;

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void hideEvent(QHideEvent* event) override;

private:
};