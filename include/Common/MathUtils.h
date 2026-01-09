#pragma once
#include "glm/glm.hpp"
#include "ProcessCraft.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include <boost/geometry.hpp>
#include <boost/polygon/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <limits>
#include <utility>
#include <algorithm>
#include <ostream>
#include <vector>
#include <cmath>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bp = boost::polygon;
typedef bg::model::d2::point_xy<float> BoostPoint;
typedef bg::model::polygon<BoostPoint> BoostPolygon;
typedef bg::model::segment<BoostPoint> BoostSegment;
typedef bg::model::linestring<BoostPoint> BoostLineString;
typedef bg::model::box<BoostPoint> BoostBox;
typedef bg::model::multi_polygon<BoostPolygon> multi_polygon;
typedef std::pair<bg::model::box<BoostPoint>, int> rtree_entry;

constexpr double PI = 3.14159265358979323846;
constexpr double deg2Rad = PI / 180.0f;
constexpr double rad2Deg = 180.0 / PI;
constexpr float epsilon = 1e-6;

#undef max
#undef min

namespace CNCSYS
{
	namespace MathUtils
	{
		float inline RadIn2PI(const glm::vec3& a, const glm::vec3& b)
		{
			float dot = glm::dot(a, b);

			float crossZ = glm::cross(a, b).z;
			float angle = deg2Rad * acos(glm::clamp(dot / (a.length() * b.length()), -1.0f, 1.f));

			if (crossZ < 0)
			{
				angle = deg2Rad * (2 * PI - angle);
			}

			return angle;
		}

		static inline float CounterClockwiseAngle(const glm::vec3& v1, const glm::vec3& v2)
		{
			float dot = glm::dot(v1, v2);
			float cross = v1.x * v2.y - v1.y * v2.x;
			float angle = std::atan2(cross, dot);

			angle = glm::degrees(angle);
			if (angle < 0)
			{
				angle += 360.0f;
			}

			return angle;
		}

		static inline float GetCosAngle(const glm::vec3& a, const glm::vec3& b)
		{
			float dotProduct = glm::dot(a, b);

			float magnitudeA = glm::length(a);
			float magnitudeB = glm::length(b);

			if (magnitudeA == 0 || magnitudeB)
				return 0;

			return dotProduct / (magnitudeA * magnitudeB);
		}

		static float GetTangentValue(const glm::vec3& a, const glm::vec3& b)
		{
			float dotProduct = glm::dot(a, b);
			float lengthA = glm::length(a);
			float lengthB = glm::length(b);

			float cosTheta = dotProduct / (lengthA * lengthB + 1e-8f);
			cosTheta = glm::clamp(cosTheta, -1.0f, 1.0f);

			float angleRadians = glm::acos(cosTheta);
			float tangent = glm::tan(angleRadians);

			return tangent;
		}

		glm::vec3 inline DeCasteljau(const std::vector<glm::vec3>& points, float t)
		{
			if (points.size() == 1)
				return points[0];

			std::vector<glm::vec3> newPoints(points.begin(), points.end() - 1);
			for (int i = 0; i < newPoints.size(); i++)
			{
				newPoints[i] = (1 - t) * points[i] + t * points[i];
			}

			return DeCasteljau(newPoints, t);
		}

		static int FindSpan(int n, int p, float t, const std::vector<float>& knots)
		{
			if (t >= knots[n + 1]) return n;
			if (t <= knots[p]) return p;

			int low = p, high = n + 1;
			int mid = (low + high) / 2;

			while (t < knots[mid] || t >= knots[mid + 1])
			{
				if (t < knots[mid]) high = mid;
				else low = mid;
				mid = (low + high) / 2;
			}

			return mid;
		}

		static glm::vec3 CalculateBSpline(const std::vector<glm::vec3>& controlPoints, const std::vector<float>& knots, int degree, float t)
		{
			int n = controlPoints.size() - 1;
			if (t <= knots[degree]) return controlPoints[0];
			if (t >= knots[n + 1]) return controlPoints[n];

			int span = FindSpan(n, degree, t, knots);

			std::vector<glm::vec3> d(degree + 1);
			for (int j = 0; j <= degree; j++)
				d[j] = controlPoints[span - degree + j];

			for (int r = 1; r <= degree; r++)
			{
				for (int j = degree; j >= r; j--)
				{
					float alpha = (t - knots[j + span - degree]) / (knots[j + 1 + span - r] - knots[j + span - degree]);
					d[j] = (1 - alpha) * d[j - 1] + alpha * d[j];
				}
			}
			return d[degree];
		}

		static std::vector<glm::vec3> BSplineToBezier(const std::vector<glm::vec3>& ctrlPts, int degree, int span, const std::vector<float>& knots)
		{
			// ctrlPts: 全部B样条控制点
			// degree: 阶数
			// span: 当前t所在的span
			// knots: 节点向量

			// 取出当前段的degree+1个控制点
			std::vector<glm::vec3> d(degree + 1);
			for (int j = 0; j <= degree; ++j)
				d[j] = ctrlPts[span - degree + j];

			// 逐步变换
			for (int r = 1; r <= degree; ++r)
			{
				for (int j = degree; j >= r; --j)
				{
					float alpha = (knots[span + j + 1 - r] - knots[span]) / (knots[span + j + 1 - r] - knots[span + j - degree]);
					d[j] = (1.0f - alpha) * d[j - 1] + alpha * d[j];
				}
			}
			// d[degree] ~ d[0] 就是Bezier段控制点
			return d;
		}

		//求样条在t处的导数(已知t所在的分段贝塞尔控制顶点和节点区间)
		static glm::vec3 BSplineDerivative(const std::vector<glm::vec3>& bezier_points,		//分段贝塞尔控制顶点
			double t, double t_j, double t_j1)  //t所在节点区间[t_j,t_j1]
		{
			int k = bezier_points.size() - 1;	//B样条次数
			if (k == 0) return glm::vec3(0.0f);

			//归一化参数u
			double u = (t - t_j) / (t_j1 - t_j);

			std::vector<glm::vec3> Q;
			for (int i = 0; i < k; ++i) {
				Q.push_back((bezier_points[i + 1] - bezier_points[i]));
			}

			glm::vec3 B_Q = DeCasteljau(Q, u);

			return B_Q * glm::vec3((k / (t_j1 - t_j)));
		}

		static double SimpsonIntegrate(const std::function<double(double)>& f, double a, double b, double eps, int depth) {

			// 增加最小区间长度检查
			const double minIntervalLength = 1e-4;
			if (b - a < minIntervalLength) {
				return (b - a) * (f(a) + f(b)) / 2; // 用梯形法近似
			}
			double c = (a + b) / 2;
			double h = b - a;
			double fa = f(a);
			double fb = f(b);
			double fc = f(c);

			double S = (h / 6) * (fa + 4 * fc + fb);
			double d = (a + c) / 2;
			double e = (c + b) / 2;
			double fd = f(d);
			double fe = f(e);
			double Sleft = (h / 12) * (fa + 4 * fd + 2 * fc + 4 * fe + fb);
			double Sright = (h / 6) * (fa + 4 * fd + fc) + (h / 6) * (fc + 4 * fe + fb);

			if (depth <= 0 || std::abs(Sleft - Sright) <= 15 * eps) {
				return Sright + (Sright - Sleft) / 15;
			}

			return SimpsonIntegrate(f, a, c, eps / 2, depth - 1) +
				SimpsonIntegrate(f, c, b, eps / 2, depth - 1);
		}

		static std::vector<float> GenerateClampedKnots(int controlPointCount, int degree)
		{
			int n = controlPointCount - 1;
			int m = n + degree + 1;
			std::vector<float> knots(m + 1);

			for (int i = 0; i <= m; i++)
			{
				if (i <= degree)
					knots[i] = 0;
				else if (i >= n + 1)
					knots[i] = 1;
				else
					knots[i] = (float)(i - degree) / (n - degree + 1);
			}
			return knots;
		}

		static int GetFirstNoneZeroDigit(float number, int& power)
		{
			if (number == 0)
				return 0;

			number = abs(number);

			int order = 0;
			if (number < 1)
			{
				while (number < 1)
				{
					number *= 10;
					order--;
				}
			}
			else
			{
				while (number >= 10)
				{
					number /= 10;
					order++;
				}
			}
			power = order;

			return (int)number;
		}

		//返回指定位数数字进位的数 1577,2 -> 1580 | 0.564,-2 -> 0.57
		static float SmallestCeilling(float value, int position)
		{
			double ret;
			if (position == 0)
				ret = ceil(value);
			else if (position > 0)
				ret = ceil((value / (pow(1, position)))) * pow(1, position);
			else
				ret = ceil((value * pow(1, position))) / pow(1, position);

			return ret;
		}

		static double GetCounterClockwiseAngle(const glm::vec3& center, const glm::vec3& start, const glm::vec3& end)
		{
			glm::vec3 v1 = start - center;
			glm::vec3 v2 = end - center;
			float dot = glm::dot(v1, v2);
			float crossZ = glm::cross(v1, v2).z;
			float angle = std::atan2(crossZ, dot);
			angle = glm::degrees(angle);
			if (angle < 0)
			{
				angle += 360.0f;
			}
			return angle;
		}

		static GeomDirection GetDirection(const glm::vec3& mid, const glm::vec3& start, const glm::vec3& end)
		{
			glm::vec3 v1 = mid - start;
			glm::vec3 v2 = end - mid;

			glm::vec3 cross = glm::cross(v1, v2);
			float len1 = glm::distance(mid, start);
			float len2 = glm::distance(start, end);
			float MaxLen = std::max(len1, len2);

			if (cross.z < 0)
				return GeomDirection::CW;
			if (cross.z > 0)
				return GeomDirection::CCW;
			if (abs(cross.z) < epsilon * MaxLen)
				return GeomDirection::AtLine;
		}


		static double NormalizeAngle(double angle)
		{
			angle = fmod(angle, PI);
			return angle < 0 ? angle + 2 * PI : angle;
		}

		static inline double ComputeArea(const std::vector<glm::vec3> polypoints)
		{
			if (polypoints.size() < 3) return 0.0; // 至少3个顶点才构成多边形
			int64_t sum = 0;
			size_t n = polypoints.size();
			for (size_t i = 0; i < n; ++i) {
				size_t j = (i + 1) % n; // 下一个顶点索引（最后一个顶点的下一个是第一个）
				sum += polypoints[i].x * polypoints[j].y - polypoints[j].x * polypoints[i].y;
			}
			return std::fabs(sum) * 0.5;
		}

		static std::tuple<glm::vec3, float, float, float> CalculateCircleByThreePoints(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
		{
			glm::vec3 center;
			float angleStart, angleEnd, radius;

			double area = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);

			double midX12 = (p1.x + p2.x) / 2.0;
			double midY12 = (p1.y + p2.y) / 2.0;

			double midX23 = (p2.x + p3.x) / 2.0;
			double midY23 = (p2.y + p3.y) / 2.0;

			double slope12 = (p2.y - p1.y) / (p2.x - p1.x);

			double slope23 = (p3.y - p2.y) / (p3.x - p2.x);

			double perpSlope12;
			if (fabs(p2.x - p1.x) < 1e-9)
			{
				perpSlope12 = 0;
			}
			else
			{
				perpSlope12 = -1 / slope12;
			}

			double perpSlope23;
			if (fabs(p3.x - p2.x) < 1e-9)
			{
				perpSlope23 = 0;
			}
			else
			{
				perpSlope23 = -1 / slope23;
			}

			if (fabs(p2.x - p1.x) < 1e-9)
			{
				center.y = (float)midY12;
				center.x = (float)(midX23 + perpSlope23 * (center.y - midY23));
			}
			else if (fabs(p3.x - p2.x) < 1e-9)
			{
				center.y = (float)midY23;
				center.x = (float)(midX12 + perpSlope12 * (center.y - midY12));
			}
			else
			{
				center.x = (float)((perpSlope12 * midX12 - midY12 - perpSlope23 * midX23 + midY23) / (perpSlope12 - perpSlope23));
				center.y = (float)(perpSlope12 * (center.x - midX12) + midY12);
			}

			radius = (float)sqrt(pow(p1.x - center.x, 2) + pow(p1.y - center.y, 2));

			double angle1 = atan2(p1.y - center.y, p1.x - center.x);
			double angle2 = atan2(p2.y - center.y, p2.x - center.x);
			double angle3 = atan2(p3.y - center.y, p3.x - center.x);

			angle1 = NormalizeAngle(angle1) * rad2Deg;
			angle2 = NormalizeAngle(angle2) * rad2Deg;
			angle3 = NormalizeAngle(angle3) * rad2Deg;

			GeomDirection dir = GetDirection(p1, p2, p3);

			if (dir == GeomDirection::CCW)
			{
				double maxAngle = std::max(angle1, angle3);
				double minAngle = std::max(angle1, angle3);
				if (angle2 > maxAngle)
				{
					if (angle1 < angle3)
					{
						angle1 = angle1 + 360;
					}
					else if (angle3 < angle1)
					{
						angle3 = angle3 + 360;
					}
				}
				if (angle2 < minAngle)
				{
					if (angle1 > angle3)
					{
						angle1 = angle1 - 360;
					}
					else
					{
						angle3 = angle3 - 360;
					}
				}
				angleStart = (float)std::min(angle1, angle3);
				angleEnd = (float)std::min(angle1, angle3);
			}
			else
			{
				double minAngle = std::min(angle1, angle3);
				double maxAngle = std::min(angle1, angle3);

				if (angle2 < minAngle)
				{
					if (angle1 > angle3)
					{
						angle1 = angle1 - 360;
					}
					else
					{
						angle3 = angle3 - 360;
					}
				}
				if (angle2 > maxAngle)
				{
					if (angle1 < angle3)
					{
						angle1 = angle1 + 360;
					}
					else
					{
						angle3 = angle3 + 360;
					}
				}
				angleStart = (float)std::max(angle1, angle3);
				angleEnd = (float)std::max(angle1, angle3);
			}

			return std::tuple<glm::vec3, float, float, float>(center, angleStart, angleEnd, radius);
		}

		static glm::vec3 CatmullRomInterpolate(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, double t, double tau = 0.5)
		{
			double t2 = t * t;
			double t3 = t2 * t;

			double x = tau * (
				(-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3 +
				(2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
				(-p0.x + p2.x) * t +
				(2 * p1.x));

			double y = tau * (
				(-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3 +
				(2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
				(-p0.y + p2.y) * t +
				(2 * p1.y));

			return glm::vec3((float)x, (float)y, 0.0f);
		}


		static std::vector<glm::vec3> CatmullRomSmooth(const std::vector<glm::vec3>& controlPoints, int segmentPerInterval = 10)
		{
			std::vector<glm::vec3> result;
			if (controlPoints.size() < 4)
			{
				return result;
			}

			glm::vec3 virtualFirst = glm::vec3(
				2 * controlPoints[0].x - controlPoints[1].x,
				2 * controlPoints[0].y - controlPoints[1].y,
				0.0f
			);

			glm::vec3 virtualLast = glm::vec3(
				2 * controlPoints[controlPoints.size() - 1].x - controlPoints[controlPoints.size() - 2].x,
				2 * controlPoints[controlPoints.size() - 1].y - controlPoints[controlPoints.size() - 2].y,
				0.0f
			);

			std::vector<glm::vec3> extendedPoints = { virtualFirst,controlPoints[0] };
			extendedPoints.insert(extendedPoints.end(), controlPoints.begin(), controlPoints.end());
			extendedPoints.push_back(virtualLast);

			for (int i = 0; i < extendedPoints.size() - 3; i++)
			{
				glm::vec3 p0 = extendedPoints[i];
				glm::vec3 p1 = extendedPoints[i + 1];
				glm::vec3 p2 = extendedPoints[i + 2];
				glm::vec3 p3 = extendedPoints[i + 3];

				for (int j = 0; j <= segmentPerInterval; j++)
				{
					double t = (double)j / segmentPerInterval;
					glm::vec3 interpolatePoint = CatmullRomInterpolate(p0, p1, p2, p3, t);
					result.push_back(interpolatePoint);
				}
			}

			return result;
		}

		typedef std::pair<glm::vec3, glm::vec3> line;

		static double PerpendicularDistance(const glm::vec3& point, line ln)
		{
			double area = abs(0.5 * (ln.first.x * ln.second.y + ln.second.x * point.y +
				point.x * ln.first.y - ln.second.x * ln.first.y -
				point.x * ln.second.y - ln.first.x * point.y));
			double bottom = sqrt(pow(ln.second.x - ln.first.x, 2));

			return area / bottom * 2;
		}

		static std::vector<glm::vec3> DouglasPeucker(std::vector<glm::vec3> points, double epsilon)
		{
			if (points.size() < 3)
				return points;

			double dmax = 0;
			int index = 0;
			line ln = std::make_pair(points[0], points[1]);

			for (int i = 1; i < points.size() - 1; i++)
			{
				double d = PerpendicularDistance(points[i], ln);
				if (d > dmax)
				{
					index = i;
					dmax = d;
				}
			}

			if (dmax > epsilon)
			{
				std::vector<glm::vec3> result1 = DouglasPeucker(std::vector<glm::vec3>(points.begin(), points.begin() + index + 1), epsilon);
				std::vector<glm::vec3> result2 = DouglasPeucker(std::vector<glm::vec3>(points.begin() + index, points.end() - 1 - index), epsilon);

				result1.erase(result1.end() - 1);
				result1.insert(result1.begin(), result2.begin(), result2.end());
				return result1;
			}
			else
			{
				return { points[0],points[points.size() - 1] };
			}
		}
		/// <summary>
		/// Get Center / StartAngle / EndAngle from Input Bulged line
		/// </summary>
		/// <param name="S">start point</param>
		/// <param name="E">end point</param>
		/// <param name="bulge">bulge</param>
		/// <returns></returns>
		static std::tuple<glm::vec3, float, float> CalculateArcParamsByBulge(const glm::vec3& S, const glm::vec3& E, double bulge)
		{
			double dx = E.x - S.x;
			double dy = E.y - S.y;
			double L = std::sqrt(dx * dx + dy * dy);

			double theta = 4 * std::atan(fabs(bulge));

			// 3. 计算圆心坐标（简化公式）
			double b = bulge;
			glm::vec3 center = glm::vec3(0.0f);
			center.x = (S.x + E.x) / 2.0 - (dy / 2.0) * (1 - b * b) / (2 * b);
			center.y = (S.y + E.y) / 2.0 + (dx / 2.0) * (1 - b * b) / (2 * b);

			// 4. 计算起始角和终止角
			double dx_s = S.x - center.x;
			double dy_s = S.y - center.y;
			double startAngle = std::atan2(dy_s, dx_s);

			double dx_e = E.x - center.x;
			double dy_e = E.y - center.y;
			double endAngle = std::atan2(dy_e, dx_e);

			// 处理顺时针情况（bulge 为负时，调整角度顺序）
			if (bulge < 0) {
				double temp = startAngle;
				startAngle = endAngle;
				endAngle = temp;
				// 确保角度差为负（顺时针）
				if (endAngle < startAngle) {
					endAngle += 2 * PI;
				}
			}
			else {
				// 逆时针时，确保 endAngle > startAngle
				if (endAngle < startAngle) {
					endAngle += 2 * PI;
				}
			}

			startAngle = startAngle * 180 / PI;
			endAngle = endAngle * 180 / PI;

			return std::tuple<glm::vec3, float, float>(center, startAngle, endAngle);
		}

		//罗德里格斯旋转公式(三维旋转)
		static glm::vec3 Rotate(const glm::vec3& vector, const glm::vec3& axis, float angle)
		{
			float angleInRad = angle * deg2Rad;
			glm::normalize(axis);
			float cosTheta = (float)cos(angleInRad);
			float sinTheta = (float)sin(angleInRad);

			return vector * cosTheta + glm::cross(axis, vector) * sinTheta + axis * glm::dot(axis, vector) * (1 - cosTheta);
		}

		static glm::vec3 Rotate(const glm::vec3& v, float angle)
		{
			float angleInRad = angle * deg2Rad;
			float cosVal = cos(angleInRad);
			float sinVal = sin(angleInRad);

			float xPrime = v.x * cosVal - v.y * sinVal;
			float yPrime = v.x * sinVal + v.y * cosVal;

			return glm::vec3(xPrime, yPrime, 0.0f);
		}

		static glm::vec3 reflectPoint(const glm::vec3& point, float A, float B, float C)
		{
			float denominator = A * A + B * B;
			if (denominator < 0.00001f) {
				throw std::invalid_argument("直线参数A和B不能同时为0");
			}

			double factor = (A * point.x + B * point.y + C) / denominator;

			glm::vec3 mirror;
			mirror.x = point.x - 2 * A * factor;
			mirror.y = point.y - 2 * B * factor;

			return glm::vec3(mirror.x, mirror.y, 0.0f);
		}

		static glm::vec3 reflectPoint(const glm::vec3& point, const glm::vec2& linePoint1, const glm::vec2& linePoint2)
		{
			if (glm::length(linePoint1 - linePoint2) < 0.00001f) {
				throw std::invalid_argument("定义直线的两点不能重合");
			}

			float A = linePoint2.y - linePoint1.y;
			float B = linePoint1.x - linePoint2.x;
			float C = linePoint2.x * linePoint1.y - linePoint1.x * linePoint2.y;

			return reflectPoint(point, A, B, C);
		}

		static glm::mat4 scaledMatrix(const glm::mat4& origin, const glm::vec3& scalar, const glm::vec3& center)
		{
			glm::vec3 offsetToOrigin = -center;

			glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
			glm::mat4 scaledMatrix = glm::scale(glm::mat4(1.0f), scalar);
			glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

			return translateBack * scaledMatrix * translateToOrigin * origin;
		}

		static glm::mat4 rotatedMatrix(const glm::mat4& origin, float angle, const glm::vec3& center)
		{
			float rotateDeg = glm::radians(angle);
			glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
			glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);
			return translateBack * rotation * translateToOrigin * origin;
		}

		static glm::mat4 tranlatedMatrix(const glm::mat4& origin, const glm::vec3& offset)
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
			return translation * origin;
		}

		static float getDeltaT(float d0, float d1, float d2, float a0, float a1, int flag)
		{
			return ((float)(d1 * (1.0f +
				1.5f * d0 * (std::max(PI / 2.0, PI - a0) / (d0 + d1)) +
				1.5f * d2 * (std::min(PI / 2.0, PI - a1) / (d1 + d2)) * flag)));
		}

		static std::vector<float> FoleyParameterize(const std::vector<glm::vec3>& points)
		{
			int n = points.size();
			std::vector<float> y = std::vector<float>(n, 0);
			if (n == 2) { y[1] = 1; return y; }

			std::vector<float> dist = std::vector<float>(n, 0);
			std::vector<float> alpha = std::vector<float>(n - 1, 0);
			for (int i = 0; i < n - 1; ++i)
				dist[i] = std::max(0.01f, glm::distance(points[i + 1], points[i]));
			for (int i = 1; i < n - 1; ++i) {
				float cosvalue = GetCosAngle(points[i - 1] - points[i], points[i + 1] - points[i]);
				alpha[i] = (float)std::min(PI - acos(cosvalue), PI / 2);
			}

			y[1] = (float)(dist[0] * (1 + 1.5 * alpha[1] * dist[1] / (dist[0] + dist[1])));
			for (int i = 2; i < n - 1; ++i)
			{
				y[i] = (float)(y[i - 1] + dist[i - 1] * (1 + 1.5 * alpha[i - 1] * dist[i - 2] / (dist[i - 2] + dist[i - 1]) + 1.5 * alpha[i] * dist[i] / (dist[i - 1] + dist[i])));
			}

			y[n - 1] = (float)(y[n - 2] + dist[n - 2] * (1 + 1.5 * alpha[n - 2] * dist[n - 3] / (dist[n - 3] + dist[n - 2])));

			for (int i = 0; i < y.size(); i++)
			{
				y[i] = y[i] / y[n - 1];
			}

			return y;
		}
	}

	// 仅通过ID删除RTree中的条目
	static bool RTree_erase_by_id(bgi::rtree<rtree_entry, bgi::quadratic<16>>& rtree, int target_id) {
		// 1. 查询所有条目，筛选出目标ID对应的条目
		std::vector<rtree_entry> candidates;
		rtree.query(bgi::satisfies([target_id](const rtree_entry& entry) {
			return entry.second == target_id; // 只匹配目标ID
			}), std::back_inserter(candidates));

		// 2. 如果找到匹配的条目，删除它
		if (!candidates.empty()) {
			// 假设ID是唯一的，删除第一个匹配的条目
			rtree.remove(candidates[0]);
			return true;
		}
		return false; // 未找到对应ID的条目
	}


	static double distance_to_polygon_boundary(const BoostPoint& point, const BoostLineString& polyline) {
		double min_distance = std::numeric_limits<double>::max(); // 初始化最大值

		for (size_t i = 0; i < polyline.size() - 1; ++i) {
			// 构建线段（多边形的一条边）
			BoostSegment segment(polyline[i], polyline[i + 1]);
			// 计算点到线段的距离
			double dist = bg::distance(point, segment);
			// 更新最小距离
			if (dist < min_distance) {
				min_distance = dist;
			}
		}
		return min_distance;
	}

	inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
	{
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, const glm::vec4& vec)
	{
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << "," << vec.w << ")";
		return os;
	}

	static struct TransformComponents {
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
	};

	static TransformComponents decomposeTransform(const glm::mat4& transform) {
		TransformComponents result;

		result.translation = glm::vec3(transform[3]);

		result.scale.x = glm::length(glm::vec3(transform[0]));
		result.scale.y = glm::length(glm::vec3(transform[1]));
		result.scale.z = glm::length(glm::vec3(transform[2]));

		if (glm::determinant(transform) < 0.0f) {
			result.scale.x *= -1;
		}

		glm::mat3 rotationMatrix(
			transform[0] / result.scale.x,
			transform[1] / result.scale.y,
			transform[2] / result.scale.z
		);
		result.rotation = glm::quat(rotationMatrix);

		return result;
	}

	// 打印拆分结果
	static void printTransformComponents(const TransformComponents& comp) {
		std::cout << "translate:" << comp.translation.x << ", "
			<< comp.translation.y << ", " << comp.translation.z << ")\n";
		std::cout << "scale: (" << comp.scale.x << ", "
			<< comp.scale.y << ", " << comp.scale.z << ")\n";
		std::cout << "rotate: (" << comp.rotation.x << ", "
			<< comp.rotation.y << ", " << comp.rotation.z << ", " << comp.rotation.w << ")\n";
	}

	static  std::ostream& operator<<(std::ostream& os, const glm::mat4& mat)
	{
		TransformComponents transform = decomposeTransform(mat);
		printTransformComponents(transform);
		return os;
	}

}
