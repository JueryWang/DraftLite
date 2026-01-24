#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <queue>
#include "glm/glm.hpp"
#include "clipper2/clipper.h"

struct Edge
{
	Edge() {}
	Edge(int _to, double _weight): to(_to),weight(_weight){}
	
	bool operator<(const Edge& other) const {
		// 按 weight 升序排列(小顶堆),若想大顶堆则改为<
		return this->weight > other.weight;
	}

	int to;
	double weight;
};

using namespace Clipper2Lib;

class VisibilityGraph {

public:
	VisibilityGraph();
	~VisibilityGraph();
	// 添加障碍物(多边形)
	void addObstacle(const Path64& polygon);
	// 添加额外的可达点（Steiner Points）
	void addExtraPoint(const Point64& p);
	//构建图结构
	void buildGraph(const Point64& start, const Point64& end);
	void ClearGraph();

	std::vector<glm::vec3> solve();
private:
	//可视性检查
	bool is_visible(const Point64& p1, const Point64& p2) const;

private:
	std::vector<Point64> nodes;
	std::vector<Path64> obstacles;
	std::vector<std::vector<std::pair<int, double>>> adj;
};