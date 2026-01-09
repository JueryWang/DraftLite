#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "Graphics/Sketch.h"
#include "Common/Shader.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/AABB.h"
#include <vector>

namespace CNCSYS
{
	class SelectionBox
	{
	public:
		SelectionBox(std::shared_ptr<SketchGPU> sketch);
		~SelectionBox();
		void ExecuteSelecting();
		std::vector<EntityVGPU*> GetSelectionItems() { return selections; }
		void Paint();

	public:
		AABB selectionRegion;
		std::vector<glm::vec3> anchorPoints;
		std::shared_ptr<SketchGPU> mSketch;
		Shader* selectBoxShader;
		GLuint vbo = 0;
		GLuint vao = 0;
		std::vector<EntityVGPU*> selections;
	};
}