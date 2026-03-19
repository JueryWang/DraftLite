#include "Graphics/DrawEntity.h"
#include "Common/MathUtils.h"
#include <QPainter>

namespace CNCSYS
{
	Point2DCPU::Point2DCPU()
	{

	}
	Point2DCPU::Point2DCPU(const glm::vec3& point)
	{
		this->point = point;
		centroid = point;
		this->bbox = new AABB(point,point);
	}
	Point2DCPU::~Point2DCPU()
	{

	}
	void Point2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->drawPoint(QPointF(point.x,point.y));
	}

	void Point2DCPU::SetParameter(const glm::vec3& pos)
	{
		this->point = point;
		bbox = AABB(point, point);
	}
	std::string Point2DCPU::ToNcInstruction()
	{
		return "";
	}

	Line2DCPU::Line2DCPU()
	{

	}
	Line2DCPU::Line2DCPU(const glm::vec3& start, const glm::vec3& end) : start(start), end(end)
	{
		bbox = AABB(start, end);

		centroid = (start + end);
		centroid /= 2;
	}
	Line2DCPU::~Line2DCPU()
	{
	}
	void Line2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setPen(QPen(this->color));
		painter->drawLine(QPointF(start.x,start.y),QPointF(end.x,end.y));
	}

	void Line2DCPU::SetParameter(const glm::vec3& start, const glm::vec3& end)
	{
		this->start = start;
		this->end = end;

		bbox = AABB(start, end);;

		area = 0;

		centroid = start + end;
		centroid /= 2;
	}
	std::string Line2DCPU::ToNcInstruction()
	{
		return "";
	}

	Arc2DCPU::Arc2DCPU()
	{
	}
	Arc2DCPU::Arc2DCPU(const glm::vec3& center, float radius, float startAngle, float endAngle) : center(center), radius(radius), startAngle(startAngle), endAngle(endAngle)
	{
		start = glm::vec3(center.x + cos(startAngle * deg2Rad), center.y + sin(startAngle * deg2Rad), 0.0f);
		end = glm::vec3(center.x + cos(endAngle * deg2Rad), center.y + sin(endAngle * deg2Rad), 0.0f);
		bbox = new AABB(start, end);

		if (glm::cross(start, end).z < 0)
		{
			std::swap(start, end);
		}

		if (startAngle > endAngle)
		{
			endAngle = endAngle + 360;
		}

		GenerateArcSamples(startAngle, endAngle, center, arcSamples);

		centroid = glm::vec3(0.0);
		for (const glm::vec3& vec : arcSamples)
		{
			//cgalPath.push_back(Point(vec.x,vec.y));
			centroid += vec;
		}

		centroid /= arcSamples.size();
	}

	Arc2DCPU::~Arc2DCPU()
	{

	}

	void Arc2DCPU::GenerateArcSamples(float startAngle, float endAngle, const glm::vec3& center, std::vector<glm::vec3>& samples)
	{
		if (samples.size())
			samples.clear();

		for (int angle = startAngle; angle <= (endAngle + 0.99f); angle++)
		{
			float x = (float)(radius * cos(angle * deg2Rad));
			float y = (float)(radius * sin(angle * deg2Rad));


			samples.push_back(glm::vec3(center.x + x, center.y + y, 0.0f));
			bbox.Union(glm::vec3(center.x, center.y, 0.0f));
		}
	}
	void Arc2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		QPainterPath path;
		painter->setPen(QPen(this->color));
		path.moveTo(QPointF(arcSamples[0].x,arcSamples[0].y));
		for (int i = 1; i < arcSamples.size(); i++)
		{
			path.lineTo(QPointF(arcSamples[i].x,arcSamples[i].y));
		}
		painter->drawPath(path);
	}

	void Arc2DCPU::SetParameter(const glm::vec3& center, float startAngle, float endAngle, float radius)
	{
		arcSamples.clear();
		//cgalPath.clear();

		area = abs((startAngle - endAngle) * deg2Rad / (PI * radius));
		start = glm::vec3(center.x + radius * (float)cos(startAngle * deg2Rad), center.y + radius * (float)sin(startAngle * deg2Rad), 0.0f);
		end = glm::vec3(center.x + radius * (float)cos(endAngle * deg2Rad), center.y + radius * (float)sin(endAngle * deg2Rad), 0.0f);

		bbox = AABB(start, end);

		glm::vec3 start = glm::vec3(start);
		glm::vec3 end3d = glm::vec(end);

		if (glm::cross(start, end).z < 0)
		{
			auto temp = start;
			start = end;
			end = temp;
		}

		if (startAngle > endAngle)
		{
			endAngle = endAngle + 360;
		}

		this->start = start;
		this->end = end;
		this->startAngle = startAngle;
		this->endAngle = endAngle;

		GenerateArcSamples(startAngle, endAngle, center, arcSamples);

		centroid = glm::vec3(0.0f);
		for (const glm::vec3& p : arcSamples)
		{
			centroid += p;
			bbox.Union(p);
		}
		centroid /= arcSamples.size();
	}
	std::string Arc2DCPU::ToNcInstruction()
	{
		return "";
	}

	Circle2DCPU::Circle2DCPU()
	{
	}
	Circle2DCPU::Circle2DCPU(const glm::vec3& center, float radius) : center(center), radius(radius)
	{
		bbox = new AABB(glm::vec3(center.x - radius, center.y - radius, 0.0f), glm::vec3(center.x + radius, center.y + radius, 0.0f));
		GenerateCircleSamplePoints(center, radius, 5, circleSamples);
		area = 2 * (float)PI * radius;

		centroid = glm::vec3(0.0f);
		for (const glm::vec3& vec : circleSamples)
		{
			//cgalPath.push_back(Point(vec.x, vec.y));
			centroid += vec;
		}
		centroid /= circleSamples.size();

		bbox = new AABB(glm::vec3(center.x - radius, center.y - radius, 0.0f), glm::vec3(center.x + radius, center.y + radius, 0.0f));
	}
	Circle2DCPU::~Circle2DCPU()
	{
	}
	void Circle2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setPen(QPen(this->color));
		QPainterPath path;
		path.moveTo(QPointF(circleSamples[0].x, circleSamples[0].y));
		for (int i = 1; i < circleSamples.size(); i++)
		{
			path.lineTo(QPointF(circleSamples[i].x,circleSamples[i].y));
		}
		painter->drawPath(path);
	}
	std::string Circle2DCPU::ToNcInstruction()
	{
		return std::string();
	}
	void Circle2DCPU::SetParameter(const glm::vec3& center, float r)
	{
		circleSamples.clear();
		//cgalPath.clear();

		GenerateCircleSamplePoints(center, radius, 10, circleSamples);
		area = 2 * (float)PI * radius;
		bbox = AABB(glm::vec3(center.x - radius, center.y - radius, 0.0f), glm::vec3(center.x + radius, center.y + radius, 0.0f));

		this->center = center;
		this->radius = r;

		centroid = glm::vec3(0.0f);
		for (const glm::vec3& vec : circleSamples)
		{
			//cgalPath.push_back(Point(vec.x, vec.y));
			centroid += vec;
		}
		centroid /= circleSamples.size();
	}

	void Circle2DCPU::GenerateCircleSamplePoints(const glm::vec3& center, float radius, int stepAngle, std::vector<glm::vec3>& samples)
	{
		if (circleSamples.size())
			circleSamples.clear();

		for (int angle = 0; angle <= 360; angle += stepAngle)
		{
			float x = (radius * cos(angle * deg2Rad));
			float y = (radius * sin(angle * deg2Rad));

			circleSamples.push_back(glm::vec3(center.x + x, center.y + y, 0.0));
		}
	}

	Ellipse2DCPU::Ellipse2DCPU()
	{
	}
	Ellipse2DCPU::Ellipse2DCPU(const glm::vec3& center, float radiusX, float radiusY) : center(center),a(radiusX),b(radiusY)
	{
		bbox = new AABB(glm::vec3(center.x - radiusX, center.y - radiusY, 0.0), glm::vec3(center.x + radiusX, center.y + radiusX, 0.0f));

		GenerateEllipseSamplePoints(center, radiusX, radiusY, 5, ellipseSamples);
	}
	Ellipse2DCPU::~Ellipse2DCPU()
	{
	}
	void Ellipse2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setPen(QPen(this->color));
		QPainterPath path;
		path.moveTo(QPointF(ellipseSamples[0].x,ellipseSamples[0].y));
		for (int i = 1; i < ellipseSamples.size(); i++)
		{
			path.lineTo(QPointF(ellipseSamples[i].x, ellipseSamples[i].y));
		}
		painter->drawPath(path);
	}
	std::string Ellipse2DCPU::ToNcInstruction()
	{
		return std::string();
	}


	void Ellipse2DCPU::SetParameter(const glm::vec3& center, float a, float b)
	{
		this->center = center;
		this->a = a;
		this->b = b;
	}
	void Ellipse2DCPU::GenerateEllipseSamplePoints(glm::vec3 center, float radiusX, float radiusY, int stepAngle, std::vector<glm::vec3>& samples)
	{
		if (samples.size() != 0)
			samples.clear();

		for (int angle = 0; angle <= 360; angle += stepAngle)
		{
			float x = radiusX * cos(angle * deg2Rad);
			float y = radiusY * sin(angle * deg2Rad);

			samples.push_back(glm::vec3(center.x + x, center.y + y, 0.0f));
		}
	}

	Polyline2DCPU::Polyline2DCPU()
	{

	}
	Polyline2DCPU::Polyline2DCPU(const std::vector<glm::vec3>& points, bool isClosed) : nodes(points)
	{
		if (nodes[0] == *(nodes.end() - 1))
			isClosed = true;

		bbox = AABB(nodes[0], nodes[1]);
		for (int i = 2; i < nodes.size(); i++)
		{
			bbox.Union(nodes[i]);
		}

		centroid = glm::vec3(0.0f);
		for (glm::vec3& p : nodes)
		{
			//cgalPath.push_back(Point(p.x,p.y));
			centroid += p;
		}
		centroid /= nodes.size();
		//area = cgalPath.area();
	}

	Polyline2DCPU::~Polyline2DCPU()
	{
	}

	void Polyline2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setPen(QPen(this->color));
		QPainterPath path;
		path.moveTo(QPointF(nodes[0].x,nodes[0].y));
		for (int i = 1; i < nodes.size(); i++)
		{
			path.lineTo(QPointF(nodes[i].x,nodes[i].y));
		}
		if (isClosed)
		{
			path.lineTo(QPointF(nodes[0].x, nodes[0].y));
		}
		painter->drawPath(path);
	}

	void Polyline2DCPU::SetParameter(const std::vector<glm::vec3>& nodes, bool isClosed)
	{
		this->nodes = nodes;
		this->isClosed = isClosed;

		//cgalPath.clear();
		if (nodes.size() > 2)
		{
			bbox = AABB(nodes[0], nodes[1]);
			for (int i = 2; i < nodes.size(); i++)
			{
				bbox.Union(nodes[i]);
			}
		}
		else if (nodes.size() >= 1)
		{
			bbox = AABB(nodes[0], nodes[0]);
		}

		centroid = glm::vec3(0.0f);
		for (const glm::vec3& p : nodes)
		{
			//cgalPath.push_back(Point(p.x, p.y));
			centroid += p;
		}
		centroid /= nodes.size();
		//area = cgalPath.area();
	}
	std::string Polyline2DCPU::ToNcInstruction()
	{
		return "";
	}

	Spline2DCPU::Spline2DCPU()
	{

	}
	Spline2DCPU::Spline2DCPU(const std::vector<glm::vec3>& controlPoints, const std::vector<float> knots, bool isPassTrough)
	{
		if (!isPassthrough)
		{
			GenerateSplineSamplePoints(controlPoints, splineSamples);
		}
		else
		{
			this->splineSamples = MathUtils::CatmullRomSmooth(controlPoints,100);
		}

		bbox = new AABB(splineSamples[0], splineSamples[1]);

		for (glm::vec3& p : splineSamples)
		{
			bbox.Union(p);
			//cgalPath.push_back(Point(p.x, p.y));
		}

		centroid = glm::vec3(0.0f);
		for (const glm::vec3& p : controlPoints)
		{
			centroid += p;
		}
		centroid /= controlPoints.size();
		//area = cgalPath.area();
	}
	Spline2DCPU::~Spline2DCPU()
	{

	}
	void Spline2DCPU::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setPen(QPen(this->color));
		QPainterPath path;
		path.moveTo(QPointF(splineSamples[0].x, splineSamples[0].y));
		for (int i = 1; i < splineSamples.size(); i++)
		{
			path.lineTo(QPointF(splineSamples[i].x, splineSamples[i].y));
		}
		painter->drawPath(path);
	}

	void Spline2DCPU::SetParameter(const std::vector<glm::vec3>& controlpoints, const std::vector<float>& knots, bool isPassthrough)
	{
		controlPoints.clear();
		splineSamples.clear();
		
		this->controlPoints = controlpoints;
		this->knots = knots;
		this->isPassthrough = isPassthrough;
		//cgalPath.clear();

		if (isPassthrough)
		{
			GenerateSplineSamplePoints(controlPoints, splineSamples);
		}
		else
		{
			splineSamples = MathUtils::CatmullRomSmooth(controlPoints, 100);
		}

		centroid = glm::vec3(0.0f);
		for (glm::vec3& p : controlPoints)
		{
			centroid += p;
			//cgalPath.push_back(Point(p.x,p.y));
		}
		centroid /= controlPoints.size();

		//area = cgalPath.area();
	}

	void Spline2DCPU::GenerateSplineSamplePoints(const std::vector<glm::vec3>& controlPoints, std::vector<glm::vec3>& samples)
	{
		if (samples.size() != 0)
			samples.clear();

		glm::vec3 firstPoint = MathUtils::CalculateBSpline(controlPoints, knots, 3, 0);
		bbox = new AABB(firstPoint, firstPoint);

		for (auto& controlpt : controlPoints)
		{
			bbox.Union(controlpt);
		}

		float step = 1.0f / controlPoints.size();
		step = step / 10.0f;

		for (float t = 0.f; t <= 1.0f; t += step)
		{
			glm::vec3 samplePoint = MathUtils::CalculateBSpline(controlPoints, knots, 3, t);
			samples.push_back(samplePoint);
		}
		glm::vec3 lastPoint = MathUtils::CalculateBSpline(controlPoints, knots, 3, 1.0f);
		samples.push_back(lastPoint);
		bbox.Union(lastPoint);
	}

	std::string Spline2DCPU::ToNcInstruction()
	{
		return "";
	}
}