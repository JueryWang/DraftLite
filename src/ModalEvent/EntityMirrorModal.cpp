#include "ModalEvent/EntityMirrorModal.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "UI/TransformBaseHint.h"
#include "Graphics/Sketch.h"
#include "Graphics/Canvas.h"

namespace CNCSYS
{
	EntityMirrorModal::EntityMirrorModal(CanvasGPU* canvas) : m_canvas(canvas)
	{
		state = ModalState::EntityMirror;
		param.mirrorTask = new MirrrorTask_param();
		line = new Line2DGPU();
	}
	EntityMirrorModal::~EntityMirrorModal()
	{
		delete param.mirrorTask;
		delete line;
	}

	void EntityMirrorModal::TransitionToProcess1(const glm::vec3& p)
	{
		param.mirrorTask->start = p;
		param.mirrorTask->end = p;
		m_canvas->mouseHint->hintLabel->setText("请选择镜面终点");
		m_canvas->mouseHint->mouseSpinX->setFocus();
		m_canvas->mouseHint->mouseSpinY->clearFocus();
		QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);
		QObject::connect(m_canvas->mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, m_canvas, [&](const glm::vec3& p) {
			this->TransitionToProcess2(p);
			});

		QObject::connect(m_canvas->mouseHint->mouseSpinY, &MouseHoverEditSpin::ConfirmData, m_canvas, [&](const glm::vec3& p) {
			this->TransitionToProcess2(p);
			});

		processStep = 1;
	}

	void EntityMirrorModal::TransitionToProcess2(const glm::vec3& p)
	{
		m_canvas->mouseHint->mouseSpinX->setFocus();
		m_canvas->mouseHint->hide();
		QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);

		param.mirrorTask->end = p;
		for (EntityVGPU* ent : m_canvas->selectedItems)
		{
			ent->Mirror(param.mirrorTask->start, param.mirrorTask->end);
			ent->UpdatePaintData();
			m_canvas->m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
		}
		m_canvas->updateGL();
		m_canvas->EndModal();

		processStep = 2;
	}

	void EntityMirrorModal::ModalMirrorEntityFunc(EntityMirrorModal* modal, CanvasGPU* canvas, const glm::vec2& viewport, const glm::mat4& projection, const glm::mat4& view)
	{
		modal->line->SetParameter(modal->param.mirrorTask->start, modal->param.mirrorTask->end);

		Shader* dashedRender = g_dashedLineShader;
		dashedRender->use();
		dashedRender->setMat4("preojection", projection);
		dashedRender->setMat4("view", view);
		dashedRender->setVec2("viewportSize", viewport);
		if (modal->processStep >= 1)
		{
			modal->line->color = assistTransformColor;
			modal->line->Paint(dashedRender, canvas->ocsSys, RenderMode::DashedLine);
		}

		Shader* mirrorShader = g_mirrorShader;
		mirrorShader->use();
		mirrorShader->setMat4("preojection", projection);
		mirrorShader->setMat4("view", view);
		mirrorShader->setVec2("viewportSize", viewport);
		for (EntityVGPU* ent : canvas->GetSelectedEntitys())
		{
			dashedRender->use();
			glm::mat4 stashWorldModel = ent->worldModelMatrix;
			ent->worldModelMatrix = ent->modelMatrixStash;
			ent->Paint(dashedRender, canvas->ocsSys, RenderMode::Line);
			ent->worldModelMatrix = stashWorldModel;

			if (modal->processStep >= 1)
			{
				mirrorShader->use();
				mirrorShader->setInt("mode", 1);
				mirrorShader->setVec2("p1", glm::vec2(modal->param.mirrorTask->start));
				mirrorShader->setVec2("p2", glm::vec2(modal->param.mirrorTask->end));
				glm::vec4 stashColor = ent->color;
				ent->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				ent->Paint(mirrorShader, canvas->ocsSys, RenderMode::Line);
				ent->color = stashColor;
				mirrorShader->setInt("mode", 0);
			}
		}
	}
}
