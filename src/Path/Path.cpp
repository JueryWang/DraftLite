#include "Path/Path.h"

Path2D::Path2D(EntityVGPU* ent_from)
{
	for (const glm::vec3& p : ent_from->GetTransformedNodes())
	{
		PathNode node;
		node.Node = p;
		tracks.push_back(node);
	}
	Idle = false;
}

Path2D::Path2D(const std::vector<glm::vec3>& nodes, bool isIdle) : Idle(isIdle)
{
	for (const glm::vec3& p : nodes)
	{
		PathNode node;
		node.Node = p;
		tracks.push_back(node);
	}
}

Path2D::~Path2D()
{
	tracks.clear();
}

std::vector<glm::vec3> Path2D::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	res.reserve(tracks.size());
	for (const PathNode& pnode : tracks)
	{
		res.push_back(worldModelMatrix * glm::vec4(pnode.Node, 1.0f));
	}
	return res;
}