#pragma once
#include "Controls/ScadaNode.h"
#include <QLabel>

class HmiTemplateLabel : public ScadaNode,public QLabel
{
	friend class ScadaScheduler;
public:
	HmiTemplateLabel(int width, int height,int fontsize = 12);
	~HmiTemplateLabel();
	virtual void UpdateNode() override;
	void SetTextPrefix(const QString& _prefix) { this->setText(_prefix); prefix = _prefix; }

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void hideEvent(QHideEvent* event) override;

private:
	QString prefix;
};