#pragma once
#include <glm/glm.hpp>
#include "ModalEvent/ModalEvent.h"

class Line2DGPU;
class Point2DGPU;

namespace CNCSYS
{
	struct MeasureDimensionModal : public ModalDrawEvent
	{
		MeasureDimensionModal(CanvasGPU* canvas);
		~MeasureDimensionModal();

		void TransitionToProcess1(const glm::vec3& p);
		void TransitionToProcess2(const glm::vec3& p);
		static void ModalMeasureDimensionFunc(MeasureDimensionModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);

		glm::vec2 point1 = glm::vec2(0.0f);
		glm::vec2 point2 = glm::vec2(0.0f);

		Line2DGPU* line = nullptr;
		Line2DGPU* lineHorizontal = nullptr;
		Line2DGPU* lineVertical = nullptr;
		bool allInited = false;

		CanvasGPU* m_canvas;
	};
}
