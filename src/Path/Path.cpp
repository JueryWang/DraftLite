#include "Path/Path.h"

Path2D::Path2D()
{
}

Path2D::Path2D(EntityVGPU* ent_from)
{
	for (const glm::vec3& p : ent_from->GetTransformedNodes())
	{
		PathNode node;
		node.Node = p;
		tracks.push_back(node);
	}
	Idle = false;
	reachedEntity.push_front(std::make_pair(ent_from,false));
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
	reachedEntity.clear();
}

void Path2D::AddPre(EntityVGPU* ent_pre,bool idle)
{
	auto nodes = ent_pre->GetTransformedNodes();
	std::reverse(nodes.begin(), nodes.end());
	for (const glm::vec3& p : nodes)
	{
		PathNode node;
		node.Node = p;
		tracks.insert(tracks.begin(), node);
	}
	reachedEntity.push_front(std::make_pair(ent_pre,idle));
}

void Path2D::AddNext(EntityVGPU* ent_next, bool idle)
{
	auto nodes = ent_next->GetTransformedNodes();
	for (const glm::vec3& p : nodes)
	{
		PathNode node;
		node.Node = p;
		tracks.push_back(node);
	}
	reachedEntity.push_back(std::make_pair(ent_next,idle));
}

std::vector<glm::vec3> Path2D::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	res.reserve(tracks.size());
	for (const PathNode& pnode : tracks)
	{
		res.push_back(glm::vec4(pnode.Node, 1.0f));
	}
	return res;
}