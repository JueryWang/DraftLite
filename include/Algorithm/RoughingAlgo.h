#pragma once
#include "clipper2/clipper.h"
#include "Graphics/AABB.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
using namespace Clipper2Lib;
//¿ª´ÖËã·¨
class RoughingAlgo
{
public:
	static std::string GetRoughingPath(EntRingConnection* shape,const AABB& workblank,RoughingParamSettings setting);
	static std::vector<Path64> GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
};