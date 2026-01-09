#pragma once
#include "Graphics/DrawEntity.h"
#include <vector>
#include <set>

using namespace CNCSYS;

class PathOptimizer
{
public:
	PathOptimizer(const std::vector<EntGroup*>& groups);
	~PathOptimizer();
	void Run();
private:
	void OptimizeGroupInnerOrder(EntGroup* group);
private:
	std::vector<EntGroup*> groups;
};