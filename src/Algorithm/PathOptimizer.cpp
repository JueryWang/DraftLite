#include "Algorithm/PathOptimizer.h"
#include "Graphics/Primitives.h"
#include <random>


PathOptimizer::PathOptimizer(const std::vector<EntGroup*>& _groups) : groups(_groups)
{
	
}

PathOptimizer::~PathOptimizer()
{
	groups.clear();
}

void PathOptimizer::Run()
{
	glm::vec3 curPos = glm::vec3(0.0f);
	std::vector<EntGroup*> UnvisitedGroups = groups;
	std::vector<EntGroup*> VisistedGroups;

	//查找零件间最近点
	int order = 0;
	while (UnvisitedGroups.size())
	{
		float minDistance = FLT_MAX;
		int bestMatchIndex = 0;
		for (int i = 0;i < UnvisitedGroups.size();i++)
		{
			glm::vec3 start = UnvisitedGroups[i]->GetProcessStartPoint();
			if (glm::distance(curPos, start) < minDistance)
			{
				bestMatchIndex = i;
				minDistance = glm::distance(curPos, start);
			}
		}
		curPos = UnvisitedGroups[bestMatchIndex]->GetProcessStartPoint();
		UnvisitedGroups[bestMatchIndex]->processOrder = order;
		VisistedGroups.push_back(UnvisitedGroups[bestMatchIndex]);
		UnvisitedGroups.erase(UnvisitedGroups.begin() + bestMatchIndex);
		order++;
	}

	for (EntGroup* visited : VisistedGroups)
	{
		OptimizeGroupInnerOrder(visited);
	}
}

void PathOptimizer::OptimizeGroupInnerOrder(EntGroup* group)
{
	//查找零件内轮廓最近点
	std::vector<EntRingConnection*> UnvisitedRings = group->rings;
	UnvisitedRings.erase(UnvisitedRings.begin());
	glm::vec3 curPos = group->GetProcessStartPoint();

	int order = 1;
	while (UnvisitedRings.size())
	{
		float minDistance = FLT_MAX;
		int bestMatchIndex = 0;
		for (int i = 0; i < UnvisitedRings.size(); i++)
		{
			glm::vec3 start = UnvisitedRings[i]->startPoint;
			if (glm::distance(curPos, start) < minDistance)
			{
				bestMatchIndex = i;
				minDistance = glm::distance(curPos, start);
			}
		}

		curPos = UnvisitedRings[bestMatchIndex]->startPoint;
		UnvisitedRings[bestMatchIndex]->processOrder = order;
		UnvisitedRings.erase(UnvisitedRings.begin() + bestMatchIndex);
		order++;
	}
}
