#pragma once
#include <QWidget>
#include <QPoint>

class TaskFlowGuide : public QWidget
{
public:
	TaskFlowGuide(QWidget* parent = nullptr);
	~TaskFlowGuide();

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
	bool isDragging = false;
	QPoint lastGlobalMousePos = QPoint(0, 0);
};