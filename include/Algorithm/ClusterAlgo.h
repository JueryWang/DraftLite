#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <utility>
#include "Graphics/DrawEntity.h"

struct CircleClusterNode
{
public:
	float x;
	float y;
	float radius;
	CNCSYS::EntityVGPU* entityParent = nullptr;
	CNCSYS::EntityVGPU* entityCentroid = nullptr;
	std::pair<int, int> indexRange;

	CircleClusterNode(CNCSYS::EntityVGPU* ent = nullptr, CNCSYS::EntityVGPU* center = nullptr,float x_ = 0.0f, float y_ = 0.0f, float radius_ = 0.0f,int indexStart = 0,int indexEnd = 0)
		: entityParent(ent), entityCentroid(center),x(x_), y(y_), radius(radius_){
		indexRange.first = indexStart;
		indexRange.first = indexEnd;
	}
};

class CircleDBSCAN
{
public:
	CircleDBSCAN(float eps, int minSamples);
	std::vector<int> Fit(const std::vector<CircleClusterNode>& circles);

private:
	// 找到给定点的所有ε-邻域内的点
	std::vector<int> GetNeighbors(const std::vector<CircleClusterNode>& circles, int circleIndex);
	//使用交并比判断圆的相似性,完全重合的圆返回值为1
	double inline CircleSimilarity(const CircleClusterNode& c1, const CircleClusterNode& c2);
	//扩展聚类
	void ExpandCluster(const std::vector<CircleClusterNode>& circles, int circleIndex, int clusterId, std::vector<int>& neighbors);
	
private:
	const float eps;					//邻域半径,相似度阈值
	const int minSamples;				//形成核心点的最小点数
	std::vector<int> labels;			//聚类标签,-1 表示噪声
	static const int UNCLASSIFIED = -2;
	static const int NOISE = -1;
};


struct PointClusterNode
{
public:
	int clippingLayer;
	glm::vec3 pt;
	CNCSYS::EntityVGPU* entityParent;
	int cluster_id = -1;

	PointClusterNode(const glm::vec3& _pt,int layer,CNCSYS::EntityVGPU* _parent) : pt(_pt), clippingLayer(layer),entityParent(_parent) {}
};

//K-Means
class PointRegionCluster
{
public:
	PointRegionCluster(const std::vector<PointClusterNode>& _points);
	~PointRegionCluster();

public:
	static std::map<int, std::vector<PointClusterNode>> kmeans(std::vector<PointClusterNode>& points,std::vector<PointClusterNode>& init_center, int k, int max_iter = 100);

private:
	std::vector<PointClusterNode> points;
};