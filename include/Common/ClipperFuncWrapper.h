#pragma once

#include <clipper2/clipper.h>
using namespace Clipper2Lib;

Paths64 GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
bool Intersect(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB);
double GetDistance(const Point64& p1, const Point64& p2);