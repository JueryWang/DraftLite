#pragma once

#include "Controls/ScadaNode.h"
#include <UI/Components/HmiInterfaceDefines.h>
#include <QPushButton>
#include <QString>

class HmiTemplateButton : public QPushButton, public ScadaNode
{
	friend class ScadaScheduler;
	Q_OBJECT
public:
	HmiTemplateButton(int width, int height,int fontsize = 16);
	~HmiTemplateButton();
	virtual void UpdateNode() override;

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void hideEvent(QHideEvent* event) override;

public slots:
	void SetPressingStyle();
	void SetReleasingStyle();

public:
	QString pressedText;
	QString releasedText;

private:
	bool isPressing = false;
};

