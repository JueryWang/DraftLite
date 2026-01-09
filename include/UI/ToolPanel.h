#pragma once

#include <QToolBar>
#include <QToolButton>

namespace CNCSYS
{
	class SketchGPU;

	class ToolPanel : public QToolBar
	{
		Q_OBJECT
	public:
		ToolPanel(std::shared_ptr<SketchGPU> sketch);
		~ToolPanel();
	private slots:
		void CreateLine();
		void CreatePolyline();
		void CreateCircle();
		void CreateArc();
		void CreateRectangle();
		void CreateSpline();
		void CreateOffset();

	private:
		std::shared_ptr<SketchGPU> mSketch;
	};
}