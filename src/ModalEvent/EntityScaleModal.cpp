#include "ModalEvent/EntityScaleModal.h"
#include "Graphics/Sketch.h"
#include "UI/TransformBaseHint.h"
#include "Graphics/Canvas.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"

namespace CNCSYS
{
	EntityScaleModal::EntityScaleModal(CanvasGPU* canvas) : m_canvas(canvas)
	{
		state = ModalState::EntityScale;
		param.scaleTask = new ScaleTask_param();
		param.scaleTask->scale = 0.0f;
		line = new Line2DGPU();
	}
	
	EntityScaleModal::~EntityScaleModal()
	{
		delete param.scaleTask;
		delete line;
	}

	void EntityScaleModal::TransitionToProcess1(const glm::vec3& p)
	{
		param.scaleTask->base = p;
		param.scaleTask->mouse = p;
		m_canvas->mouseHint->hintLabel->setText("请指定缩放比例");
		m_canvas->mouseHint->mouseSpinX->setValue(0.0f);
		m_canvas->mouseHint->mouseSpinX->setFocus();
		m_canvas->mouseHint->mouseSpinY->clearFocus();
		m_canvas->mouseHint->mouseSpinY->setVisible(false);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);
		QObject::connect(m_canvas->mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, m_canvas, [&](const glm::vec3& p) {
				m_canvas->mouseHint->mouseSpinX->setFocus();
				m_canvas->mouseHint->hide();
				QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
				QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);
				float scale = m_canvas->mouseHint->mouseSpinX->value();

				for (EntityVGPU* ent : m_canvas->selectedItems)
				{
					ent->Scale(glm::vec3(scale, scale, scale),param.scaleTask->selectionBox.Center());
					ent->UpdatePaintData();
					m_canvas->m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
				}

				m_canvas->updateGL();
				m_canvas->EndModal();
		});

		processStep = 1;
	}

	void EntityScaleModal::TransitionToProcess2(const glm::vec3& p)
	{
		param.scaleTask->mouse = p;

		float scaleAnchor = param.scaleTask->selectionBox.MinRange() / 2.0f;
		float hoverDistance = glm::distance(param.scaleTask->mouse, param.scaleTask->base);
		param.scaleTask->scale = hoverDistance / scaleAnchor;
		float scale = param.scaleTask->scale;
		m_canvas->mouseHint->mouseSpinX->setValue(scale);

		glm::vec3 centroid = param.scaleTask->selectionBox.Center();

		for (EntityVGPU* ent : m_canvas->selectedItems)
		{
			ent->Scale(glm::vec3(scale, scale, scale), centroid);
			ent->UpdatePaintData();
			m_canvas->m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
		}
		m_canvas->updateGL();
		m_canvas->EndModal();
		processStep = 2;
	}

	void EntityScaleModal::ModalScaleEntityFunc(EntityScaleModal* modal, CanvasGPU* canvas, const glm::vec2& viewport, const glm::mat4& projection, const glm::mat4& view)
	{
		modal->line->SetParameter(modal->param.scaleTask->base,modal->param.scaleTask->mouse);

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

		Shader* solidRender = g_lineShader;
		solidRender->use();
		solidRender->setMat4("preojection", projection);
		solidRender->setMat4("view", view);
		solidRender->setVec2("viewportSize", viewport);
		for (EntityVGPU* ent : canvas->GetSelectedEntitys())
		{
			dashedRender->use();
			glm::mat4 stashWorldModel = ent->worldModelMatrix;
			ent->worldModelMatrix = ent->modelMatrixStash;
			ent->Paint(dashedRender, canvas->ocsSys, RenderMode::Line);
			ent->worldModelMatrix = stashWorldModel;
			solidRender->use();
			glm::vec4 stashColor = ent->color;
			ent->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			ent->Paint(solidRender, canvas->ocsSys, RenderMode::Line);
			ent->color = stashColor;
		}
	}
}