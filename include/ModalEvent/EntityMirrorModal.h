#pragma once
#include <glm/glm.hpp>
#include "ModalEvent/ModalEvent.h"

class Line2DGPU;

namespace CNCSYS
{
	struct EntityMirrorModal : public ModalDrawEvent
	{
		EntityMirrorModal(CanvasGPU* canvas);
		~EntityMirrorModal();

		void TransitionToProcess1(const glm::vec3& p);
		void TransitionToProcess2(const glm::vec3& p);
		static void ModalMirrorEntityFunc(EntityMirrorModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
		
		Line2DGPU* line = nullptr;
		CanvasGPU* m_canvas;
	};
}