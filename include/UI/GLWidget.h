#pragma once
#include "Common/OpenGLContext.h"
#include <functional>
#include <QKeyEvent>
#include <unordered_map>
#include <QWidget>
#include <QTimer>
#include <Graphics/AxisTicker.h>
#include <Graphics/OCS.h>

#include <vector>

//标注结构体
struct Tag
{
public:
	Tag(const glm::vec3& _point, const QString& _label, int _size) : pixelPos(_point),label(_label),size(_size)
	{

	}
	glm::vec3 pixelPos;
	QString label;
	int size;
};

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
		CanvasGPU* GetCanvas();
		void AddTag(const Tag& tag) { canvasTags.push_back(tag); }
		OpenGLWindowContext* GetContet() { return m_context; }
		void update();
	protected:
		void paintEvent(QPaintEvent* event) override;

	public:
		SketchGPU* attachedSketch = nullptr;

	private:
		OpenGLWindowContext* m_context;
		OCSGPU* ocsSys = nullptr;
		QTimer m_updateTimer;

		bool middleBtnPressing = false;
		QPointF previousMousePos;

		std::vector<Tag> canvasTags;

		QFont font;
	};
}