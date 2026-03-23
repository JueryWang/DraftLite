#include "Graphics/SelectionBox.h"
#include "Graphics/OCS.h"

namespace CNCSYS
{
	SelectionBox::SelectionBox(std::shared_ptr<SketchGPU> sketch) : mSketch(sketch)
	{
		selectBoxShader = new Shader("Shader/drawSelectionBox.vert", "Shader/drawSelectionBox.frag");
	}

	SelectionBox::~SelectionBox()
	{
		mSketch.reset();
		delete selectBoxShader;

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (vao != 0)
		{
			glDeleteVertexArrays(1, &vao);
			vao = 0;
		}
		if (vbo != 0)
		{
			glDeleteBuffers(1, &vbo);
			vbo = 0;
		}
	}

	void SelectionBox::ExecuteSelecting()
	{

	}

	void SelectionBox::Paint()
	{
		float vertices[] = {
		selectionRegion.min.x,selectionRegion.max.y,0.0f,
		selectionRegion.min.x,selectionRegion.min.y,0.0f,
		selectionRegion.max.x,selectionRegion.max.y,0.0f,
		selectionRegion.max.x,selectionRegion.min.y,0.0f
		};
		if (vao == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		selectBoxShader->use();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);

		glm::mat4 ortho = g_canvasInstance->GetOCSSystem()->camera->GetOrthoGraphicMatrix();
		glm::mat4 view = g_canvasInstance->GetOCSSystem()->camera->GetViewMatrix();
		selectBoxShader->setMat4("projection", ortho);
		selectBoxShader->setMat4("view", view);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
}