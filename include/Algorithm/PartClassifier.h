#pragma once

#include <Graphics/DrawEntity.h>
#include <vector>
#include "clipper2/clipper.h"

using namespace CNCSYS;

//零件识别算法
class PartClassifier
{
public:
	PartClassifier(const std::vector<EntRingConnection*>& _rings);
	~PartClassifier();
	std::vector<EntGroup*> Execute();
private:
	bool inline IsPolygonContains(const Clipper2Lib::Path64& child, const Clipper2Lib::Path64& parent);
private:
	std::vector<EntRingConnection*> rings;
	std::map<EntRingConnection*, Clipper2Lib::Path64*> pathMapper;
	std::vector<EntGroup*> partList;
};