#include "ModalEvent/EntityRotateModal.h"
#include "Common/MathUtils.h"
#include "Graphics/Sketch.h"
#include "UI/TransformBaseHint.h"
#include "Graphics/Canvas.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"

namespace CNCSYS
{
	EntityRotateModal::EntityRotateModal(CanvasGPU* canvas) : m_canvas(canvas)
	{
		state = ModalState::EntityRotate;
		param.rotateTask = new RotateTask_param();
		param.rotateTask->angle = 0;
		param.rotateTask->deltaAngle = 0;
		param.rotateTask->base = glm::vec3(0.0f);
		param.rotateTask->startPoint = glm::vec3(0.0f);
		param.rotateTask->endPoint = glm::vec3(0.0f);
		ln_baseStart = new Line2DGPU();
		ln_baseEnd = new Line2DGPU;
	}
	EntityRotateModal::~EntityRotateModal()
	{
		delete param.rotateTask;
		delete ln_baseStart;
		delete ln_baseEnd;
	}

	void EntityRotateModal::TransitionToProcess1(const glm::vec3& p)
	{
		param.rotateTask->base = p;
		param.rotateTask->startPoint = p;
		processStep = 1;
		m_canvas->mouseHint->hintLabel->setText("请指定旋转起始点");
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
	}

	void EntityRotateModal::TransitionToProcess2(const glm::vec3& p)
	{
		m_canvas->mouseHint->hintLabel->setText("请指定旋转角");
		m_canvas->mouseHint->mouseSpinY->setVisible(false);
		m_canvas->mouseHint->repaint();
		param.rotateTask->startPoint = p;
		param.rotateTask->endPoint = p;
		processStep = 2;
		m_canvas->mouseHint->mouseSpinX->setFocus();
		m_canvas->mouseHint->mouseSpinY->clearFocus();

		QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);
		QObject::connect(m_canvas->mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, m_canvas, [&](const glm::vec3& p) {
				EntityRotateModal* rotateModal = static_cast<EntityRotateModal*>(m_canvas->modalEv);
				rotateModal->param.rotateTask->angle = m_canvas->mouseHint->mouseSpinX->value();
				m_canvas->mouseHint->mouseSpinX->setFocus();
				m_canvas->mouseHint->hide();
				QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
				QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);
				for (EntityVGPU* ent : m_canvas->selectedItems)
				{
					ent->Rotate(param.rotateTask->base, rotateModal->param.rotateTask->angle);
					ent->UpdatePaintData();
					m_canvas->m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
				}
				m_canvas->updateGL();
				m_canvas->EndModal();
			});
	}

	void EntityRotateModal::TransitionToProcess3(const glm::vec3& p)
	{
		//增量式
		m_canvas->mouseHint->mouseSpinX->setFocus();
		QObject::disconnect(m_canvas->mouseHint->mouseSpinX, nullptr, nullptr, nullptr);
		QObject::disconnect(m_canvas->mouseHint->mouseSpinY, nullptr, nullptr, nullptr);

		m_canvas->mouseHint->hide();
		param.rotateTask->endPoint = p;
		float angle = MathUtils::CounterClockwiseAngle(param.rotateTask->startPoint - param.rotateTask->base, param.rotateTask->endPoint - param.rotateTask->base);
		float deltaRotate = angle - param.rotateTask->angle;
		param.rotateTask->angle = angle;

		for (EntityVGPU* ent : m_canvas->selectedItems)
		{
			ent->Rotate(param.rotateTask->base, deltaRotate);
			ent->UpdatePaintData();
			m_canvas->m_currentSketch.get()->UpdateEntityBox(ent,ent->bbox);
		}
		m_canvas->updateGL();
		m_canvas->EndModal();

		processStep = 3;
	}


	void EntityRotateModal::ModalRotateEntityFunc(EntityRotateModal* modal, CanvasGPU* canvas, const glm::vec2& viewport, const glm::mat4& projection, const glm::mat4& view)
	{
		modal->ln_baseStart->SetParameter(modal->param.rotateTask->base, modal->param.rotateTask->startPoint);
		modal->ln_baseEnd->SetParameter(modal->param.rotateTask->base, modal->param.rotateTask->endPoint);

		Shader* dashedRender = g_dashedLineShader;
		dashedRender->use();
		dashedRender->setMat4("preojection", projection);
		dashedRender->setMat4("view",view);
		dashedRender->setVec2("viewportSize",viewport);
		if (modal->processStep >= 1)
		{
			modal->ln_baseStart->color = assistTransformColor;
			modal->ln_baseStart->Paint(dashedRender, canvas->ocsSys, RenderMode::DashedLine);
		}
		if (modal->processStep >= 2)
		{
			modal->ln_baseEnd->color = assistTransformColor;
			modal->ln_baseEnd->Paint(dashedRender, canvas->ocsSys, RenderMode::DashedLine);
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