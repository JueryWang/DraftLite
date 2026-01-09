#pragma once
#include "glad/glad.h"
#include "ModalEvent/ModalEvent.h"

class Line2DGPU;

namespace CNCSYS
{

	class CanvasGPU;

	struct EntityRotateModal : public ModalDrawEvent
	{
	public:
		EntityRotateModal(CanvasGPU* canvas);
		~EntityRotateModal();

		void TransitionToProcess1(const glm::vec3& p);
		void TransitionToProcess2(const glm::vec3& p);
		void TransitionToProcess3(const glm::vec3& p);
		static void ModalRotateEntityFunc(EntityRotateModal*, CanvasGPU*,const glm::vec2& viewport, const glm::mat4&, const glm::mat4&);

		Line2DGPU* ln_baseStart = nullptr;
		Line2DGPU* ln_baseEnd = nullptr;
		CanvasGPU* m_canvas;
	};


}