#include "Algorithm/CircleDetector.h"
#define M_PI   3.14159265358979323846

CircleDBSCAN::CircleDBSCAN(float _eps, int _minSamples) : eps(_eps),minSamples(_minSamples)
{

}

std::vector<int> CircleDBSCAN::Fit(const std::vector<CircleClusterNode>& circles)
{
	int clusterId = 0;
	labels = std::vector<int>(circles.size(), UNCLASSIFIED);

	for (int i = 0; i < circles.size(); i++)
	{
		if (labels[i] == UNCLASSIFIED)
		{
			std::vector<int> neighbors = GetNeighbors(circles, i);

			if (neighbors.size() < minSamples)
			{
				labels[i] = NOISE;
			}
			else
			{
				ExpandCluster(circles, i, clusterId, neighbors);
				clusterId++;
			}
		}
	}

	return labels;
}

std::vector<int> CircleDBSCAN::GetNeighbors(const std::vector<CircleClusterNode>& circles, int circleIndex)
{
	std::vector<int> neighbors;

	for (int i = 0; i < circles.size(); i++)
	{
		if (i == circleIndex)
			continue;

		double similarity = CircleSimilarity(circles[circleIndex],circles[i]);
		if (similarity > eps)
			neighbors.push_back(i);
	}

	return neighbors;
}

double CircleDBSCAN::CircleSimilarity(const CircleClusterNode& c1, const CircleClusterNode& c2)
{
	float dx = c1.x - c2.x;
	float dy = c1.y - c2.y;
	float distance = (float)(sqrt(dx * dx + dy * dy));

	if (distance <= fabs(c1.radius - c2.radius))
		return 0.0f;

	if (distance <= fabs(c1.radius - c2.radius))
	{
		float smallerRadius = std::min(c1.radius,c2.radius);
		float largerRadius = std::max(c1.radius,c2.radius);
		return (smallerRadius * smallerRadius) / (largerRadius * largerRadius);
	}
	// 计算相交面积
	float r1 = c1.radius;
	float r2 = c2.radius;
	float d = distance;

	float part1 = r1 * r1 * static_cast<float>(acos((d * d + r1 * r1 - r2 * r2) / (2 * d * r1)));
	float part2 = r2 * r2 * static_cast<float>(acos((d * d + r2 * r2 - r1 * r1) / (2 * d * r2)));
	float part3 = 0.5f * static_cast<float>(sqrt((-d + r1 + r2) * (d + r1 - r2) * (d - r1 + r2) * (d + r1 + r2)));

	float intersectionArea = part1 + part2 - part3;
	float unionArea = static_cast<float>(M_PI) * (r1 * r1 + r2 * r2) - intersectionArea;

	return intersectionArea / unionArea;
}

void CircleDBSCAN::ExpandCluster(const std::vector<CircleClusterNode>& circles, int circleIndex, int clusterId, std::vector<int>& neighbors)
{
	labels[circleIndex] = clusterId;

	int index = 0;
	while (index < neighbors.size())
	{
		int currentIndex = neighbors[index];

		if (labels[currentIndex] == UNCLASSIFIED)
		{
			std::vector<int> currentNeighbors = GetNeighbors(circles, currentIndex);

			//若当前点是核心点,将其邻域点加入待处理列表(去重)
			if (currentNeighbors.size() >= minSamples)
			{
				std::unordered_set<int> neighborSet(neighbors.begin(), neighbors.end());
				for (int n : currentNeighbors)
				{
					if (neighborSet.find(n) == neighborSet.end())
					{
						neighbors.push_back(n);
						neighborSet.insert(n);
					}
				}
			}
		}

		if (labels[currentIndex] == UNCLASSIFIED || labels[currentIndex] == NOISE) {
			labels[currentIndex] = clusterId;
		}

		index++;
	}
}
