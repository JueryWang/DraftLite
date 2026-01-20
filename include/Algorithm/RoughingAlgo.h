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
	static Polyline2DGPU* GetRoughingPath(EntRingConnection* shape,const AABB& workblank,double toolRadius = 10,double stepover = 5, double allowance = 0.05);
	static std::vector<Path64> GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
};