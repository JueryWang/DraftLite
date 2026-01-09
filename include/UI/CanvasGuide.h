#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>

class CanvasGuide : public QWidget
{
	Q_OBJECT
public:
	static CanvasGuide* GetInstance();

protected:
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;

private:
	CanvasGuide(QWidget* parent = nullptr);
	~CanvasGuide();

public:
	QPushButton* btnViewUp = nullptr;
	QPushButton* btnViewDown = nullptr;
	QPushButton* btnViewLeft = nullptr;
	QPushButton* btnViewRight = nullptr;
	QPushButton* btnZoomIn = nullptr;
	QPushButton* btnZoomOut = nullptr;

	QTimer* longPressTimerViewUp = nullptr;
	QTimer* longPressTimerViewDown = nullptr;
	QTimer* longPressTimerViewLeft = nullptr;
	QTimer* longPressTimerViewRight = nullptr;
	QTimer* longPressTimerZoomIn = nullptr;
	QTimer* longPressTimerZoomOut = nullptr;

private:
	static CanvasGuide* s_instance;
};