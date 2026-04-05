#include "Graphics/ContourTree.h"
#include <algorithm>
#define PRECISION 1000

bool IsPathFullyContained(const Path64& outer,const Path64& inner)
{
	for (const auto& pt : inner) {
		if (Clipper2Lib::PointInPolygon(pt, outer) == Clipper2Lib::PointInPolygonResult::IsOutside) {
			return false;
		}
	}
	return true;
}

ContourTreeNode::ContourTreeNode()
{
	
}
ContourTreeNode::ContourTreeNode( EntRingConnection* ring) :contour(ring)
{
	if (ring)
	{
		for (const glm::vec3& p : ring->contour)
		{
			clipperPath.emplace_back(p.x * PRECISION,p.y * PRECISION);
		}
	}
	area = ring->area;
}

ContourTree::ContourTree(std::vector<EntRingConnection*>& inputs, int drawingWidth, int drawingHeight)
{
	root = new ContourTreeNode();
	root->parent = nullptr;
	root->clipperPath.emplace_back(-drawingWidth * PRECISION, -drawingHeight * PRECISION);
	root->clipperPath.emplace_back(drawingWidth * PRECISION, -drawingHeight * PRECISION);
	root->clipperPath.emplace_back(drawingWidth * PRECISION, drawingHeight * PRECISION);
	root->clipperPath.emplace_back(-drawingWidth * PRECISION, drawingHeight * PRECISION);
	root->clipperPath.emplace_back(-drawingWidth * PRECISION, -drawingHeight * PRECISION);
	root->contour = nullptr;
	root->layerDepth = 0;

	std::sort(inputs.begin(), inputs.end(), [](EntRingConnection* cn1, EntRingConnection* cn2) {return cn1->area > cn2->area; });
	sortedNodes.insert(root);

	for (EntRingConnection* contour : inputs)
	{
		ContourTreeNode* node = new ContourTreeNode(contour);
		InsertConour(node);
	}
	sortedNodes;
}

ContourTree::~ContourTree()
{
	for (auto* it : sortedNodes)
	{
		delete it;
	}
}

void ContourTree::InsertConour(ContourTreeNode* node)
{
	for (auto it = sortedNodes.begin(); it != sortedNodes.end(); ++it)
	{
		ContourTreeNode* candidate = *it;
		if (IsPathFullyContained(candidate->clipperPath, node->clipperPath))
		{
			node->parent = candidate;
			node->layerDepth = candidate->layerDepth + 1;
			node->contour->depth = node->layerDepth;
			candidate->childs.push_back(node);

			sortedNodes.insert(node);  // O(log n) 插入
			return;
		}
	}
}
