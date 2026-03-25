#pragma once
#include "Graphics/DrawEntity.h"
#include "clipper2/clipper.h"
#include <queue>
#include <functional>

using namespace CNCSYS;
using namespace Clipper2Lib;

struct ContourTreeNode
{
public:
	ContourTreeNode();
	ContourTreeNode(EntRingConnection* ring);

	bool operator<(const ContourTreeNode* other) const
	{
		return layerDepth > other->layerDepth;
	}

	int layerDepth = 0;
	double area;
	ContourTreeNode* parent = nullptr;
	std::vector<ContourTreeNode*> childs;
	Clipper2Lib::Path64 clipperPath;
	EntRingConnection* contour;
};

struct LayerComp {
	bool operator()(ContourTreeNode* a, ContourTreeNode* b) const {
		// 先按 layerNum 排序，layerNum 相同按指针地址（保证严格弱序）
		if (a->layerDepth != b->layerDepth)
		{
			return a->layerDepth > b->layerDepth;
		}
		else
		{
			return fabs(a->area) < fabs(b->area);
		}
		return a < b;  // 指针地址作为第二排序键
	}
};

class ContourTree
{
public:
	ContourTree(std::vector<EntRingConnection*>& inputs, int drawingWidth, int drawingHeight);
	~ContourTree();

private:
	void InsertConour(ContourTreeNode* node);

private:
	ContourTreeNode* root = nullptr;
	std::set<ContourTreeNode*, LayerComp> sortedNodes;

};