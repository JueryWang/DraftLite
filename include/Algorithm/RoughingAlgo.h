#pragma once
#include "clipper2/clipper.h"
#include "Graphics/AABB.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Algorithm/VisibilityGraph.h"
#include "Algorithm/ClusterAlgo.h"
using namespace Clipper2Lib;

//¿ª´ÖËã·¨
class RoughingAlgo
{
public:
	static std::string GetRoughingPath(EntRingConnection* shape,AABB& workblank,RoughingParamSettings setting);
	static std::map<int, std::vector<PointClusterNode>> GetRegionResult() { return regionSet; }
	static std::string RequestRegion(const std::vector<int>& groupNumbers, RegionParamSettings setting);

private:
	static inline double GetDistance(const Point64& p1, const Point64& p2);
	static void ProbePath(const Point64& start,const Point64& end,const Path64& collisionShape,int probeDistance,std::vector<Point64>& intermidate, int deepth);
	static void TrimPath(std::vector<Point64>& path,const Path64& collisionShape);
	static Point64 GenRandomPointInCircle(int xc, int yc, int r);

	static std::vector<CNCSYS::EntityVGPU*> s_cache;
	static int percision;
	static std::map<int, std::vector<PointClusterNode>> regionSet;
	static std::map<int,Path2D*> regionPaths;
};