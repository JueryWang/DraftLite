#pragma once

#include "Graphics/AABB.h"
#include "Common/Shader.h"
#include "Common/Context.h"
#include "Common/OpenGLContext.h"
#include "UI/OpenGLRenderWindow.h"
#include "Graphics/OCS.h"
#include "ModalEvent/EntityRotateModal.h"
#include "ModalEvent/EntityScaleModal.h"
#include <QPoint>
#include <Windows.h>
#include <QWindow>
#include <QWidget>
#include <QGraphicsView>
#include <vector>
#include <functional>
#include <map>
#include <QImage>
#include <QRect>

#define CONNECT_EPSILON 0.05

constexpr glm::vec4 assistTransformColor = glm::vec4(227.0f / 255.0f, 192.0f / 255.0f, 141.0f / 255.0f, 1.0f);

class Polyline2DGPU;
class Point2DGPU;
class Anchor;
class CanvasGuide;
class Text;

namespace CNCSYS
{
	class EntityVGPU;
	class EntityVCPU;
	class OCSGPU;
	class OCSCPU;
	class SketchGPU;
	class SketchCPU;
	class GLWidget;
	class EntRingConnection;
	class Polyline2DCPU;
	class SelectionBox;
	class TransformBaseHint;
	class GLWidget;
	struct EntityPoint;
}

namespace CNCSYS
{
	enum CaptureMode
	{
		Entity,
		Section,
		Point
	};

	class CanvasGPU : public OpenGLRenderWindow
	{
		friend class SketchGPU;
		friend struct EntityRotateModal;
		friend struct EntityScaleModal;
		friend struct EntityMirrorModal;
		friend struct EntityShiftModal;
		friend struct MeasureDimensionModal;

		Q_OBJECT
	public:
		CanvasGPU(std::shared_ptr<SketchGPU> sketch,OCSGPU* ocs,int width, int height, bool isMainCanvas);
		~CanvasGPU();

		void UpdateOCS();
		OCSGPU* GetOCSSystem() { return ocsSys; }
		std::shared_ptr<SketchGPU> GetSketchShared() { return m_currentSketch; }
		std::vector<EntityVGPU*> GetSelectedEntitys() { return selectedItems; }
		void SetRenderMode(RenderMode mode) { this->renderMode = mode; }
		void SetFrontWidget(GLWidget* frontWidget);
		void ModalEventDraw(const glm::vec2& viewport, const glm::mat4& view, const glm::mat4& projection);
		void EnterModal(ModalState modal);
		void EndModal();
		void CancelModal();
		void EraseSelectedEntitys();
		void ResetCanvas();
		void SetScene(std::shared_ptr<SketchGPU> sketch, OCSGPU* ocs);
		void SetCaptureMode(CaptureMode mode);
		GLWidget* GetFrontWidget() { return frontWidget; }
		QImage GrabImage(SketchGPU* sketch, OCSGPU* ocs, int imageWidth, int imageHeight,const glm::vec4& bgcolor = glm::vec4(1.0,1.0,1.0,1.0));

	protected:
		virtual bool eventFilter(QObject* obj, QEvent* event) override;
		virtual void updateGL() override;

	private:
		void DrawTickers();
		void handleEventCreateEntity(GLWidget* glwgt, QObject* obj, QEvent* event);
		void handleEventSelection(GLWidget* glwgt, QObject* obj, QEvent* event);
		void handleEventCapture(GLWidget* glwgt, const QPointF& mousePos);
		void handleEventEntityMove(GLWidget* glwgt, QEvent* event);
		void handleEventEntityRoatate(GLWidget* glwgt, QEvent* event);
		void handleEventEntityScale(GLWidget* glwgt, QEvent* event);
		void handleEventEntityMirror(GLWidget* glwgt, QEvent* event);
		void handleEventShortCut(GLWidget* glwgt, QEvent* event);//快捷键处理
		void handleEventMeasureDimension(GLWidget* glwgt, QEvent* event);//测量模态处理
		void PushToHistory();

	private slots:
		void OnComfirmCreateElements();
		void OnCancelCreateElements();
		void OnPaceLeft();
		void OnPaceRight();
		void OnPaceUp();
		void OnPaceDown();
		void OnZoomIn();
		void OnZoomOut();

	public:
		std::function<void(const QString&)> hoverChangedCallback;
		glm::vec4 background = glm::vec4(20.0f / 255.0f, 20.0f / 255.0f, 41.0f / 255.0f, 1.0f);
		bool showArrow = false;
		bool showInnerPoint = false;
		bool drawTickers = true;
		ModalState operationState = ModalState::NormalInteract;

	private:
		CaptureMode captureType = CaptureMode::Entity;
		RenderMode renderMode = RenderMode::Line;

		GLWidget* frontWidget = nullptr;
		OCSGPU* ocsSys = nullptr;
		SelectionBox* selectBox = nullptr;
		AABB lastSelectionRegion;
		Shader* tickerTextRenderShader = nullptr;
		Shader* tickerRuleRenderShader = nullptr;
		std::shared_ptr<SketchGPU> m_currentSketch;

		std::vector<EntityVGPU*> selectedItems;
		std::vector<EntityVGPU*> assistantEnt;

		Point2DGPU* hoverPoint;
		QPoint lastMousePos;
		QPoint offsetMove;
		bool firstResize;
		bool isDragging = false;
		bool isSelecting = false;
		bool isMainCanvas;

		GLuint tickerRenderVAO;
		GLuint tickerRenderVBO;
		GLuint textRenderVAO;
		GLuint textRenderVBO;

		//捕捉精度
		float captureRadius = 1.0f;
		EntityVGPU* lastHoverEntity = nullptr;

		std::map<ModalState, std::function<void(GLWidget* glwgt, QEvent* event)>> modalCallbacks;
		std::vector<glm::vec3> modalClickPos;
		EntityVGPU* tempEntityCreated = nullptr;
		TransformBaseHint* mouseHint = nullptr;

		ModalDrawEvent* modalEv = nullptr;
		Anchor* toolAnchor = nullptr;
		QRect guideRegion = QRect(50, 10, 100, 60);

		std::pair<EntityPoint*, EntityPoint*> measurement;//测量标注
	};

	class CanvasCPU : public QGraphicsView
	{
		friend class SketchCPU;
	public:
		CanvasCPU(std::shared_ptr<SketchCPU> sketch, int width, int height, bool isMainCanvas);
		~CanvasCPU();

		void UpdateOCS();
		void ResetView();

		void EnterModal(ModalState modal);
		void EndModal();

	protected:
		virtual bool eventFilter(QObject* obj, QEvent* event) override;

	private:
		OCSCPU* ocsSys = nullptr;
		std::shared_ptr<SketchCPU> m_sketch;
		QGraphicsScene* m_scene;
		QPoint lastMousePos;

		std::vector<EntRingConnection*> parts;
		bool isDragging;
		bool isMainCanvas;

		ModalState operationState = ModalState::NormalInteract;
		std::vector<glm::vec3> modalClickPos;
		EntityVCPU* tempEntityCreated = nullptr;
	};
}