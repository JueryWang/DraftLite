#pragma once
#include<QMainWindow>
#include<QLabel>
#include<QMenuBar>
#include<QMenu>
#include<QAction>
#include<QString>
#include<QTimer>
#include"Graphics/Canvas.h"
#include "UI/GLWidget.h"
class MenuLayerTop;
class GCodeEditor;

using namespace CNCSYS;
class OverallWindow;

class MainLayer : public QMainWindow
{
	friend class MenuLayerTop;
public:
	MainLayer(void* sketch, OverallWindow* ovWindow);
	~MainLayer();
	QWidget* GetCanvasPanel() { return canvasOperationPanel; }
	QWidget* GetWebView() { return webView; }
	void loadQssTheme();
	CNCSYS::SketchGPU* GetSketch() { return mSketchGPU.get(); }
	void SetSketch(std::shared_ptr<CNCSYS::SketchGPU> sketch) { mSketchGPU = sketch; }

	QSize GetWindowSize();
	void AutoSaveScene();
	void UpdateSize();

protected:
	void changeEvent(QEvent* event) override;
	virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
	bool renderByGPU = true;
	std::shared_ptr<CNCSYS::SketchGPU> mSketchGPU;
	std::shared_ptr<CNCSYS::SketchGPU> mSketchCPU;
	OCSGPU* mOCSGPU;
	std::shared_ptr<GLWidget> mGLWidget;
	QLabel* m_bottomInfo;
	QString m_qssPath;

	MenuLayerTop* menu;
	GLWidget* glWidget;
	QWidget* canvasOperationPanel;
	QWidget* webView;
	GCodeEditor* editor;
};