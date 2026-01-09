#pragma once
#include "Common/OpenGLContext.h"
#include <functional>
#include <QKeyEvent>
#include <unordered_map>
#include <QWidget>
#include <QTimer>
#include <Graphics/AxisTicker.h>
#include <Graphics/OCS.h>

namespace CNCSYS
{
	class OpenGLRenderWindow;
	class SketchGPU;
	class OCSGPU;

	class GLWidget : public QWidget
	{
		friend class CanvasGPU;
		Q_OBJECT
	public:
		GLWidget(OpenGLRenderWindow* glwindow, SketchGPU* sketch, WindowState state = STATIC_DRAW);
		~GLWidget();

		void SetWindowStatus(WindowState state);
		void SetOCSystem(OCSGPU* _ocsSys) { ocsSys = _ocsSys; }
		OpenGLWindowContext* GetContet() { return m_context; }
		void update();

	protected:
		void paintEvent(QPaintEvent* event) override;

	private:
		OpenGLWindowContext* m_context;
		OCSGPU* ocsSys = nullptr;
		QTimer m_updateTimer;

		bool middleBtnPressing = false;
		QPointF previousMousePos;

		QFont font;
	};
}