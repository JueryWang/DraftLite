#include "Graphics/Anchor.h"
#include "Graphics/AABB.h"
#include "Graphics/Canvas.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "Common/Program.h"
#include "UI/GCodeEditor.h"
#include "UI/GLWidget.h"

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
	if (animatorOpen)
	{
		if (vao == 0)
		{
			glGenVertexArrays(1,&vao);
			glGenBuffers(1, &vbo);
		}

		if (animatorPath.capacity() == 0)
		{
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);
		}
	}
	else
	{
		pathIndex = 0;
		animatorPath.clear();
		glDeleteBuffers(1,&vbo);
		vbo = 0;
	}

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double> diff = now - last_update_time;
	long long diff_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	
	queueLocker.lock();
	static int lastRow = 0;
	int currentRowCNC = 0;

	PLC_TYPE_DWORD cycleTime;
	ReadPLC_OPCUA(g_ConfigableKeys["AnimatorCycleTime"].c_str(),&cycleTime,AtomicVarType::DWORD);
	cycleTime /= 1000;

	for(int i = 0; i < ceil((float)diff_milliseconds/ cycleTime);i++)
	{
		if (!pointQueue.empty())
		{
			currentRowCNC = pointQueue.front().iCurrentRow;
			animatorPath.push_back(glm::vec3(pointQueue.front().fX,pointQueue.front().fY,pointQueue.front().fZ));
			pointQueue.pop();
		}
	}

	if (currentRowCNC != lastRow)
	{
		QMetaObject::invokeMethod(GCodeEditor::GetInstance(), "SetMarkLine", Qt::QueuedConnection, Q_ARG(int, currentRowCNC));
		lastRow = currentRowCNC;
	}

	queueLocker.unlock();
	last_update_time = now;
}

void Anchor::ReadFromQueueBuffer(int index, int length)
{
	//读取到本地队列
	static int lastReadIndex = index;
	if (index != lastReadIndex)
	{
		if (index == 0)
		{
			queueLocker.lock();
			for (int i = 1; i < length+1;i++)
			{
				if (g_simRecBufferA[i].fX != 0 || g_simRecBufferA[i].fY != 0)
				{
					pointQueue.push(g_simRecBufferA[i]);
				}
			}
			queueLocker.unlock();
		}
		else
		{
			queueLocker.lock();
			for (int i = 1; i < length+1;i++)
			{
				if (g_simRecBufferB[i].fX != 0 || g_simRecBufferB[i].fY != 0)
				{
					pointQueue.push(g_simRecBufferB[i]);
				}
			}
			queueLocker.unlock();
		}
		lastReadIndex = index;
	}
}

void Anchor::Paint()
{
	const glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
	float half = ocsSys->canvasRange->MinRange() * 0.03f;

	if (animatorPath.size())
	{
		position = animatorPath.back();
	}
	crossline1->SetParameter(glm::vec3(position.x - half, position.y + half, 1.0f), glm::vec3(position.x + half, position.y - half, 1.0f));
	crossline1->Paint(g_lineShader, ocsSys, RenderMode::Line);
	crossline2->SetParameter(glm::vec3(position.x + half, position.y + half, 1.0f), glm::vec3(position.x - half, position.y - half, 1.0f));
	crossline2->Paint(g_lineShader, ocsSys, RenderMode::Line);

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

		glClear(GL_DEPTH_BUFFER_BIT);

		glPointSize(4);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); // 先置空
		// 3. 上传当前帧完整数据（使用GL_DYNAMIC_DRAW，适配频繁更新）
		glBufferData(GL_ARRAY_BUFFER, animatorPath.size() * sizeof(glm::vec3),
			animatorPath.data(), GL_DYNAMIC_DRAW);

		// 确保 vertex attrib pointer 已设置
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);
		// 绘制所有已添加的顶点
		glDrawArrays(GL_LINE_STRIP, 0, animatorPath.size());

		glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑VBO
		glBindVertexArray(0);
		glPointSize(2);
	}
}

void Anchor::CleanCache()
{
	while (pointQueue.size())
	{
		pointQueue.pop();
	}
	animatorPath.clear();
}
