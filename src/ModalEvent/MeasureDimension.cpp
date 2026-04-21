#include "ModalEvent/MeasureDimension.h"
#include "Graphics/Line2D.h"
#include "Graphics/Point2D.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Sketch.h"
#include "Graphics/OCS.h"
#include "Graphics/Canvas.h"
namespace CNCSYS
{
	MeasureDimensionModal::MeasureDimensionModal(CanvasGPU* canvas)
	{
		state = ModalState::MeasureDimension;
		param.measureTask = new MeasureDimensionTask_param();
		line = new Line2DGPU();
		lineHorizontal = new Line2DGPU();
		lineVertical = new Line2DGPU();
	}

	MeasureDimensionModal::~MeasureDimensionModal()
	{
		delete param.measureTask;
		delete lineHorizontal;
		delete lineVertical;
	}

	void MeasureDimensionModal::TransitionToProcess1(const glm::vec3& p)
	{

	}

	void MeasureDimensionModal::TransitionToProcess2(const glm::vec3& p)
	{

	}

	void MeasureDimensionModal::ModalMeasureDimensionFunc(MeasureDimensionModal* modal, CanvasGPU* canvas, const glm::vec2& viewport, const glm::mat4& projection, const glm::mat4& view)
	{
		modal->line->SetParameter(glm::vec3(modal->point1, 0.0f), glm::vec3(modal->point2, 0.0f));

		glm::vec3 pointRightAng = glm::vec3(modal->point2.x, modal->point1.y, 0.0f);
		modal->lineHorizontal->SetParameter(glm::vec3(modal->point1, 0.0f), pointRightAng);
		modal->lineVertical->SetParameter(pointRightAng, glm::vec3(modal->point2, 0.0f));

		glm::vec2 horizontalTextPos = glm::vec2((modal->point1.x + pointRightAng.x) / 2, (modal->point1.y));
		glm::vec2 verticalTextPos = glm::vec2((pointRightAng.x), (pointRightAng.y + modal->point2.y) / 2);
		glm::vec2 diagonalTextPos = glm::vec2((modal->point1.x + modal->point2.x) / 2, (modal->point1.y + modal->point2.y) / 2);
		std::string horizontalText = std::to_string(glm::distance(glm::vec3(modal->point1, 0.0f), pointRightAng));
		std::string verticalText = std::to_string(glm::distance(pointRightAng, glm::vec3(modal->point2, 0.0f)));
		std::string diagonalText = std::to_string(glm::distance(glm::vec3(modal->point1, 0.0f), glm::vec3(modal->point2, 0.0f)));

		float product = glm::dot(modal->point2 - modal->point1, glm::vec2(1.0, 0));
		float angle = acos(product / (glm::length(modal->point2 - modal->point1) * glm::length(glm::vec2(1.0, 0))));
		float angleDeg = glm::degrees(angle);
		std::string angleText = std::to_string(angleDeg);
		size_t dotPos = angleText.find('.');
		if (dotPos != std::string::npos) {
			if (angleText.size() > dotPos + 2)
			{
				angleText = angleText.substr(0, dotPos + 2);
			}
		}
		angleText = std::string(QString("Angle: ").toLocal8Bit()) + angleText;

		Shader* dashedShader = g_dashedLineShader;
		dashedShader->use();
		dashedShader->setMat4("projection", projection);
		dashedShader->setMat4("view", view);

		Shader* textShader = canvas->tickerTextRenderShader;
		textShader->use();
		textShader->setMat4("projection", projection);
		textShader->setMat4("view", view);

		float scale = canvas->ocsSys->scale;
		if (modal->allInited)
		{
			modal->lineVertical->color = g_yellowColor;
			modal->lineVertical->Paint(dashedShader, canvas->ocsSys, RenderMode::DashedLine);
			modal->lineHorizontal->color = g_yellowColor;
			modal->lineHorizontal->Paint(dashedShader, canvas->ocsSys, RenderMode::DashedLine);
			modal->line->color = g_yellowColor;
			modal->line->Paint(dashedShader, canvas->ocsSys, RenderMode::DashedLine);
		}
	}
}
