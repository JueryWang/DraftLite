#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "Graphics/OCS.h"
#include "Graphics/Line2D.h"
#include "Controls/ScadaNode.h"

using namespace CNCSYS;

class Anchor : public ScadaNode
{
public:
	Anchor();
	~Anchor();
	void SetCoordinateSystem(OCSGPU* ocsSystem);
	void SetCurrentCanvas(CanvasGPU* _canvas)
	{
		canvas = _canvas;
	}
	void SetPosition(const glm::vec3& pos);
	void Paint();

protected:
	virtual void UpdateNode() override;

public:
	
	std::chrono::steady_clock::time_point last_update_time;
	bool animatorOpen = true;
	int pathIndex = 0;

private:
	static Anchor* instance;

	GLuint vao = 0;
	GLuint vbo = 0;

	CanvasGPU* canvas;
	OCSGPU* ocsSys;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	int animUpdateBatchSize = 8;
	Line2DGPU* crossline1 = nullptr;
	Line2DGPU* crossline2 = nullptr;
	std::vector<glm::vec3> animatorPath;
	std::queue<glm::vec3> pointQueue;
	std::mutex queueLocker;
};