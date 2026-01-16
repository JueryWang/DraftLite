#include "Graphics/Anchor.h"
#include "Graphics/AABB.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "Common/ProgressInfo.h"
#include "UI/GCodeEditor.h"

Anchor* Anchor::instance = nullptr;

Anchor* Anchor::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new Anchor();
		ScadaScheduler::GetInstance()->AddNode(instance);
	}

	return instance;
}

Anchor::Anchor()
{
	crossline1 = new Line2DGPU();
	crossline1->color = g_redColor;
	crossline2 = new Line2DGPU();
	crossline2->color = g_redColor;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
}

void Anchor::SetCoordinateSystem(OCSGPU* ocsSystem)
{
	ocsSys = ocsSystem;
}

Anchor::~Anchor()
{

}

void Anchor::ResetAnimation()
{
	pathIndex = 0;
	animatorOpen = false;
}

void Anchor::SetPosition(const glm::vec3& pos)
{
	position = pos;
}

void Anchor::UpdateNode()
{
	//static glm::vec3 lastPosition = position;

	//PLCParam_ProtocalOpc* XposInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[g_ConfigableKeys["AxisX"]]);
	//PLCParam_ProtocalOpc* YposInfo = static_cast<PLCParam_ProtocalOpc*>(g_PLCVariables[g_ConfigableKeys["AxisY"]]);

	//if (XposInfo && YposInfo)
	//{
	//	AtomicVar<PLC_TYPE_LREAL>* varX = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(XposInfo->bindVar);
	//	AtomicVar<PLC_TYPE_LREAL>* varY = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(YposInfo->bindVar);

	//	position.x = varX->GetValue();
	//	position.y = varY->GetValue();

	//	if (animatorOpen)
	//	{
	//		// 初始化 VAO/VBO（只在首次需要时）
	//		if (vao == 0)
	//		{
	//			glGenVertexArrays(1, &vao);
	//			glGenBuffers(1, &vbo);
	//		}

	//		// 预分配容量（如果还没有分配）
	//		if (animatorPath.capacity() == 0)
	//		{
	//			animatorPath = std::vector<glm::vec3>(GCodeEditor::GetInstance()->lines());
	//			glBindVertexArray(vao);
	//			glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//			glBufferData(GL_ARRAY_BUFFER, (animatorPath.size()) * sizeof(glm::vec3), animatorPath.data(), GL_DYNAMIC_DRAW);

	//			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	//			glEnableVertexAttribArray(0);
	//		}

	//		// 当位置变化时，添加新路径点
	//		if (lastPosition != position)
	//		{
	//			animatorPath[pathIndex] = position;
	//			pathIndex++;
	//			lastPosition = position;
	//		}

	//	}
	//}

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
	}
	glLineWidth(2);
	glBindVertexArray(0);
}

void Anchor::ReAssignDataSize(int newSize)
{
	if (animatorPath.size() != newSize)
	{
		if (vbo <= 0)
		{
			glGenBuffers(1, &vbo);
		}
		animatorPath.clear();
		animatorPath = std::vector<glm::vec3>(newSize);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, (animatorPath.size()) * sizeof(glm::vec3), animatorPath.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		pathIndex = 0;
	}

	position = glm::vec3(0, 0, 0);
}
