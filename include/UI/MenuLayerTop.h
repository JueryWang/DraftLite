#pragma once
#include <QWidget>
#include <QMenuBar>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QAction>
#include <QSettings>

class OverallWindow;

class MenuLayerTop : public QWidget
{
	Q_OBJECT
public:
	MenuLayerTop(OverallWindow* parent = NULL);
	~MenuLayerTop();

public slots:
	void OnImportDxf();
	void OnImportDwg();
	void OnExportNC();
	void OnExportScene();
	void OnImportScene();

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;\

private:
	void ImportDxf(const QString& dxfFile);

private:
	QMenuBar* topMenus;
	QMenu* fileMenu;
	QMenu* editMenu;
	QMenu* captureMenu;
	QHBoxLayout* layout;

	OverallWindow* ovWindow;

	QAction* actEntityCapture = nullptr;
	QAction* actPointCapture = nullptr;
	QAction* actShowArrow = nullptr;
	QAction* actConfiExport = nullptr;
	QAction* actAuthInformation = nullptr;
	QAction* actShowInnerPoint = nullptr;

	bool m_isDraging = false;
	QPoint m_offsetPoint = QPoint(0, 0);

	QGroupBox* AuthInformation = nullptr;
};
