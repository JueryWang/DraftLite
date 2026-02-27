#pragma once

#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <clipper2/clipper.h>
using namespace Clipper2Lib;

struct RRTNode{
	Point64 point;
	int parent_id;
	RRTNode(const Point64& pt, int pid = -1) : point(pt), parent_id(pid) {}
};

class RRT {
public:
	RRT(const Point64& start, const Point64& end);
	void AddObstacle(const Path64& collisionShape);
	void SetObstacle(const Path64& collisionShape);
	void ProbePath(std::vector<Point64>& intermidate,int probeDistance,const Point64& MapMin,const Point64& MapMax);

private:
	int FindNearesNodeIndex(const std::vector<RRTNode>& nodePool, const Point64& target);
	Point64 ExtendNode(const Point64& source, const Point64& target, int stepLength);
	inline bool IsPointInArea(const Point64& point, int minX, int maxX, int minY, int maxY);
	void BacktrackPath(const std::vector<RRTNode>& nodePool,
		int endNodeIndex,
		std::vector<Point64>& path);
private:
	Point64 startPos;
	Point64 goalPos;
	Paths64 obstacles;

	int maxNodeCount = 10000;
};