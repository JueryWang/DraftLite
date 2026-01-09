#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Graphics/AABB.h"

namespace CNCSYS
{
	class Camera2D
	{
	public:
		Camera2D(AABB* bbox) : range(bbox)
		{

		}

		void UpdateRange(AABB* range)
		{
			*this->range = *range;
		}

		glm::mat4 GetOrthoGraphicMatrix()
		{
			return glm::ortho(range->min.x, range->max.x, range->min.y, range->max.y);
		}

		glm::mat4 GetViewMatrix()
		{
			return glm::mat4(1.0f);
		}
	private:
		AABB* range;
	};
}
