#include "Graphics/Anchor.h"
#include "Graphics/AABB.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "Common/ProgressInfo.h"
#include "UI/GCodeEditor.h"

#define AnimateUpdateBatchSize 8

Anchor* Anchor::instance = nullptr;

Anchor::Anchor()
{
	crossline1 = new Line2DGPU();
	crossline1->color = g_redColor;
	crossline2 = new Line2DGPU();
	crossline2->color = g_redColor;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	last_update_time = std::chrono::steady_clock::now();
}

void Anchor::SetCoordinateSystem(OCSGPU* ocsSystem)
{
	ocsSys = ocsSystem;
}

Anchor::~Anchor()
{
}

void Anchor::SetPosition(const glm::vec3& pos)
{
	position = pos;
}

void Anchor::UpdateNode()
{
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double> diff = now - last_update_time;
	long long diff_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

	PLC_TYPE_INT cycle;
	ReadPLC_OPCUA(g_ConfigableKeys["AnimatorCycleTime"].c_str(), &cycle, AtomicVarType::INT);
	int timeInterval = (AnimateUpdateBatchSize * cycle);

	if (diff_milliseconds > timeInterval)
	{
		PLC_TYPE_INT bufferALength;
		ReadPLC_OPCUA(g_ConfigableKeys["AnimatorBufferLengthQueueA"].c_str(), &bufferALength, AtomicVarType::INT);
		if (bufferALength > 0)
		{
			
		}
	}
}

void Anchor::Paint()
{
	const glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
	float half = ocsSys->canvasRange->MinRange() * 0.03f;

	crossline1->SetParameter(glm::vec3(position.x - half, position.y + half, 1.0f), glm::vec3(position.x + half, position.y - half, 1.0f));
	crossline1->Paint(g_lineShader, ocsSys, RenderMode::Line);
	crossline2->SetParameter(glm::vec3(position.x + half, position.y + half, 1.0f), glm::vec3(position.x - half, position.y - half, 1.0f));
	crossline2->Paint(g_lineShader, ocsSys, RenderMode::Line);

	static bool firstUpdate = true;

	if (animatorOpen && !animatorPath.empty())
	{
		g_lineShader->use();

		// 设置必要的 shader uniform（与其他 Paint 函数保持一致）
		glm::mat4 ortho = ocsSys->camera->GetOrthoGraphicMatrix();
		glm::mat4 view = ocsSys->camera->GetViewMatrix();
		g_lineShader->setMat4("projection", ortho);
		g_lineShader->setMat4("view", view);
		g_lineShader->setMat4("model", glm::mat4(1.0f));
		g_lineShader->setVec4("PaintColor", g_yellowColor);

		glLineWidth(4);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, animatorPath.size() * sizeof(glm::vec3), animatorPath.data(), GL_DYNAMIC_DRAW);

		// 确保 vertex attrib pointer 已设置
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);

		// 绘制所有已添加的顶点
		glDrawArrays(GL_LINE_STRIP, 0, pathIndex);

		glLineWidth(2);
		glBindVertexArray(0);
	}
}