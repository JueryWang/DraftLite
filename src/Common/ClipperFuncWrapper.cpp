#include "Common/ClipperFuncWrapper.h"

Paths64 GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB)
{
    //获取碰撞截断结果
    Paths64 subject;
    subject.push_back(pathA);

    Paths64 clip;
    clip.push_back(pathB);

    Clipper64 clipper;

    clipper.AddOpenSubject(subject);
    clipper.AddClip(clip);

    Paths64 solution_closed;
    Paths64 solution_open;
    clipper.Execute(ClipType::Intersection, FillRule::NonZero, solution_closed, solution_open);
    return solution_open;
}

bool Intersect(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB)
{
    bool collision = (GetIntersections(pathA, pathB).size() > 0);
    for (const Point64& vertex : pathA)
    {
        if (PointInPolygon(vertex, pathB) == PointInPolygonResult::IsInside)
        {
            collision = true;
        }
    }
    return collision;
}


double GetDistance(const Point64& p1, const Point64& p2)
{
    double dx = static_cast<double>(p1.x - p2.x);
    double dy = static_cast<double>(p1.y - p2.y);

    return sqrt(dx * dx + dy * dy);
}
