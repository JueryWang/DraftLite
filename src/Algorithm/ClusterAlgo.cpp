#include "Algorithm/ClusterAlgo.h"
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
	double dx = static_cast<double>(c1.x) - c2.x;
	double dy = static_cast<double>(c1.y) - c2.y;
	double d = sqrt(dx * dx + dy * dy);

	double r1 = c1.radius;
	double r2 = c2.radius;

	// 1. 完全分离 (纠正：应该是半径之和)
	if (d >= r1 + r2) return 0.0;

	// 2. 包含关系
	if (d <= fabs(r1 - r2))
	{
		double rSmall = std::min(r1, r2);
		double rLarge = std::max(r1, r2);
		return (rSmall * rSmall) / (rLarge * rLarge);
	}

	// 3. 相交逻辑 (只有在这里才执行)
	double r1Sq = r1 * r1;
	double r2Sq = r2 * r2;

	// 关键：防止 acos 越界
	double cos1 = std::max(-1.0, std::min(1.0, (d * d + r1Sq - r2Sq) / (2 * d * r1)));
	double cos2 = std::max(-1.0, std::min(1.0, (d * d + r2Sq - r1Sq) / (2 * d * r2)));

	double part1 = r1Sq * acos(cos1);
	double part2 = r2Sq * acos(cos2);
	// 关键：防止 sqrt 负数
	double part3 = 0.5 * sqrt(std::max(0.0, (-d + r1 + r2) * (d + r1 - r2) * (d - r1 + r2) * (d + r1 + r2)));

	double intersectionArea = part1 + part2 - part3;
	double unionArea = M_PI * (r1Sq + r2Sq) - intersectionArea;

	return intersectionArea / unionArea;
}

void CircleDBSCAN::ExpandCluster(const std::vector<CircleClusterNode>& circles, int circleIndex, int clusterId, std::vector<int>& neighbors)
{
	labels[circleIndex] = clusterId;

	// 1. 在循环外维护哈希集合，避免重复创建
	std::unordered_set<int> inSeedSet(neighbors.begin(), neighbors.end());

	// 2. 这里的 index 模拟了队列的操作
	int index = 0;
	while (index < neighbors.size())
	{
		int currentIndex = neighbors[index];

		if (labels[currentIndex] == NOISE) {
			labels[currentIndex] = clusterId;
		}
		else if (labels[currentIndex] == UNCLASSIFIED) {
			labels[currentIndex] = clusterId;

			std::vector<int> currentNeighbors = GetNeighbors(circles, currentIndex);

			if (currentNeighbors.size() >= minSamples) {
				for (int n : currentNeighbors) {
					if (inSeedSet.insert(n).second) {
						neighbors.push_back(n);
					}
				}
			}
		}
		index++;
	}
}

PointRegionCluster::PointRegionCluster(const std::vector<PointClusterNode>& _points) : points(_points)
{

}

PointRegionCluster::~PointRegionCluster()
{

}

std::map<int, std::vector<PointClusterNode>> PointRegionCluster::kmeans(std::vector<PointClusterNode>& points,std::vector<PointClusterNode>& init_center, int k, int max_iter)
{
	std::map<int, std::vector<PointClusterNode>> cluster_sets;
	std::vector<glm::vec3> centers;

	for (int i = 0; i < init_center.size(); i++)
	{
		init_center[i].cluster_id = i;
		centers.push_back(init_center[i].pt);
	}

	int n_points = points.size();
	if (k <= 0 || k > n_points) {
		std::cerr << "Invalid k value! k must be between 1 and " << n_points << std::endl;
		return cluster_sets;
	}
	// 2.1 分配每个点到最近的簇
	bool converged = false;
	int iter = 0;
	while (!converged && iter < max_iter) {
		converged = true;

		for (auto& p : points) {
			double min_dist = INFINITY;
			int best_cluster = 0;
			for (int i = 0; i < k; ++i)
			{
				double dist = glm::distance(p.pt,centers[i]);
				if (dist < min_dist)
				{
					min_dist = dist;
					best_cluster = i;
				}
			}
			if (p.cluster_id != best_cluster) {
				p.cluster_id = best_cluster;
				converged = false;
			}
		}

		//更新聚类中心
		std::vector<glm::vec3> new_centers(k, glm::vec3());
		std::vector<int> cluster_counts(k, 0);
		for (const auto& p : points)
		{
			int cid = p.cluster_id;
			new_centers[cid] += p.pt;
			cluster_counts[cid]++;
		}

		for (int i = 0; i < k; ++i) {
			if (cluster_counts[i] > 0) {
				new_centers[i] /= cluster_counts[i];
			}
			// 检查中心是否变化（阈值1e-6避免浮点误差）
			if (glm::distance(new_centers[i],centers[i]) > 1e-6) {
				converged = false;
			}
		}
		centers = new_centers;
		iter++;
	}

	for (const auto& p : points)
	{
		cluster_sets[p.cluster_id].push_back(p);
	}

	return cluster_sets;
}


