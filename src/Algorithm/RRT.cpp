#include "Algorithm/RRT.h"
#include "Common/ClipperFuncWrapper.h"
RRT::RRT(const Point64& start, const Point64& end) : startPos(start),goalPos(end)
{
}

void RRT::ProbePath(std::vector<Point64>& intermediate, int probeDistance, const Point64& MapMin, const Point64& MapMax)
{
	std::vector<RRTNode> nodePool;
	nodePool.emplace_back(startPos,-1);

	std::uniform_int_distribution<int> distX(MapMin.x, MapMax.x);
	std::uniform_int_distribution<int> distY(MapMin.y, MapMax.y);
	std::random_device rd;
	std::mt19937 gen(rd());

	bool findPath = false;
	int endNodeIndex = -1;
	while (nodePool.size() < maxNodeCount && !findPath)
	{
		Point64 randomPoint;
		if (rand() % 10 == 0) {
			randomPoint = goalPos;
		}
		else
		{
			randomPoint = Point64(distX(gen),distY(gen));
		}

		int nearestNodeIndex = FindNearesNodeIndex(nodePool, randomPoint);
		RRTNode nearestNode = nodePool[nearestNodeIndex];

		Point64 newNodePoint = ExtendNode(nearestNode.point, randomPoint, probeDistance);
		Path64 marchingLine;
		marchingLine.emplace_back(nearestNode.point.x, nearestNode.point.y);
		marchingLine.emplace_back(newNodePoint.x, newNodePoint.y);
		bool hasCollision = false;
		bool isValidNode = IsPointInArea(newNodePoint, MapMin.x, MapMax.x, MapMin.x, MapMax.x);

		for (Path64& obstacle : obstacles)
		{
			hasCollision |= Intersect(marchingLine,obstacle);
		}

		if (!hasCollision && isValidNode) {
			nodePool.emplace_back(newNodePoint,nearestNodeIndex);
			int newNodeIndex = nodePool.size() - 1;

			if (GetDistance(newNodePoint, goalPos) <= probeDistance)
			{
				findPath = true;
				endNodeIndex = newNodeIndex;
				break;
			}
		}
	}

	if (findPath && endNodeIndex != -1) {
		BacktrackPath(nodePool, endNodeIndex, intermediate);
		intermediate.emplace_back(goalPos.x, goalPos.y);
	}
}

int RRT::FindNearesNodeIndex(const std::vector<RRTNode>& nodePool, const Point64& target)
{
	int nearestIndex = 0;
	double minDistance = Distance(nodePool[0].point, target);
	for (int i = 1; i < nodePool.size(); i++)
	{
		double currentDistance = Distance(nodePool[i].point,target);
		if (currentDistance < minDistance)
		{
			minDistance = currentDistance;
			nearestIndex = i;
		}
	}
	return nearestIndex;
}

Point64 RRT::ExtendNode(const Point64& source, const Point64& target, int stepLength)
{
	if (source == target) return source;

	long long dx = target.x - source.x;
	long long dy = target.y - source.y;

	double length = sqrt(dx * dx + dy * dy);
	double ratio = stepLength / length;
	long long newX = source.x + static_cast<long long>(dx * ratio);
	long long newY = source.y + static_cast<long long>(dy * ratio);
	return Point64(newX, newY);
}

inline bool RRT::IsPointInArea(const Point64& point, int minX, int maxX, int minY, int maxY)
{
	return (point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY);
}

void RRT::BacktrackPath(const std::vector<RRTNode>& nodePool, int endNodeIndex, std::vector<Point64>& path)
{
	int currentIndex = endNodeIndex;
	// 从终点节点回溯到根节点（父节点索引为 -1）
	while (currentIndex != -1) {
		path.emplace_back(nodePool[currentIndex].point.x, nodePool[currentIndex].point.y);
		currentIndex = nodePool[currentIndex].parent_id;
	}
	// 反转路径，得到从起点到终点的正序路径
	std::reverse(path.begin(), path.end());
}

void RRT::AddObstacle(const Path64& collisionShape)
{
	obstacles.push_back(collisionShape);
}

void RRT::SetObstacle(const Path64& collisionShape)
{
	obstacles.clear();
	obstacles.push_back(collisionShape);
}
