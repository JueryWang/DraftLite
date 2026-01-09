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
	static Anchor* GetInstance();
	void SetCoordinateSystem(OCSGPU* ocsSystem);
	void SetPosition(const glm::vec3& pos);
	void Paint();
	void ReAssignDataSize(int newSize);
	void ResetAnimation();

protected:
	virtual void UpdateNode() override;

private:
	Anchor();
	~Anchor();
	void UpdateOnePoint(const glm::vec3& newPoint);
public:
	bool animatorOpen = true;

private:
	static Anchor* instance;

	GLuint vao = 0;
	GLuint vbo = 0;

	OCSGPU* ocsSys;
	glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f);

	Line2DGPU* crossline1 = nullptr;
	Line2DGPU* crossline2 = nullptr;
	std::vector<glm::vec3> animatorPath;
	int pathIndex = 0;
};