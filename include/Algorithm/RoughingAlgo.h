#pragma once
#include "clipper2/clipper.h"
#include "Graphics/AABB.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Algorithm/VisibilityGraph.h"
using namespace Clipper2Lib;

//开粗算法
class RoughingAlgo
{
public:
	static std::string GetRoughingPath(EntRingConnection* shape,const AABB& workblank,RoughingParamSettings setting);
	static Paths64 GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
	static bool Intersect(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
private:
	//空走路径碰到工件,则插点回避碰撞
	static void InterpToEscape(const glm::vec3 start,const glm::vec3 end, VisibilityGraph& vGraph,std::string& gcode);
	static inline double GetDistance(const Point64& p1, const Point64& p2);
	static void ProbePath(Point64& start,const Point64& end,const Path64& collisionShape,int probeDistance,std::vector<Point64>& intermidate, int deepth);
	static void TrimPath(std::vector<Point64>& path,const Path64& collisionShape);
	static Point64 GenRandomPointInCircle(int xc, int yc, int r);

	static std::vector<CNCSYS::EntityVGPU*> s_cache;
	static int percision;
};