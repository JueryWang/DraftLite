#pragma once
#include<QMainWindow>
#include<QLabel>
#include<QMenuBar>
#include<QMenu>
#include<QAction>
#include<QString>
#include<QTimer>
#include<QTabWidget>
#include"Graphics/Canvas.h"
#include "UI/GLWidget.h"
#include "UI/SketchInformation.h"
#include "UI/Components/HmiTemplateStationPreview.h"
class MenuLayerTop;
class GCodeEditor;

using namespace CNCSYS;
class OverallWindow;

class MainLayer : public QMainWindow
{
	friend class MenuLayerTop;
public:
	MainLayer(OverallWindow* ovWindow);
	~MainLayer();
	QWidget* GetCanvasPanel() { return canvasOperationPanel; }
	QWidget* GetWebView() { return webView; }
	void loadQssTheme();
	CNCSYS::SketchGPU* GetSketch() { return mSketchGPU.get(); }
	std::shared_ptr<CNCSYS::SketchGPU> GetSketchShared() { return mSketchGPU; }

	QSize GetWindowSize();
	void AutoSaveScene();
	void UpdateSize();

protected:
	void changeEvent(QEvent* event) override;
	virtual bool eventFilter(QObject* obj, QEvent* event) override;

public:
	bool renderByGPU = true;
	//画布默认的Sketch和OCS,不可修改
	std::shared_ptr<CNCSYS::SketchGPU> mSketchGPU;
	std::shared_ptr<CNCSYS::SketchGPU> mSketchCPU;
	OCSGPU* mOCSGPU;
	QString m_qssPath;

	MenuLayerTop* menu = nullptr;
	GLWidget* preview = nullptr;
	QWidget* canvasOperationPanel = nullptr;
	QWidget* webView = nullptr;
	GCodeEditor* editor = nullptr;
	SketchInfoPanel* infoPanel = nullptr;
	StationSwitchTab* stationTab = nullptr;
	QHBoxLayout* previewLayout = nullptr;

	std::vector<std::shared_ptr<SketchGPU>> sketchLists;
	int currentSketchIndex = 0;
};