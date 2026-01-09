#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "Common/OpenGLContext.h"
#include "Graphics/AABB.h"
#include <QString>
#include <string>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp> // 支持vector序列化
#include <QGraphicsItem>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::d2::point_xy<float> BoostPoint;
typedef bg::model::polygon<BoostPoint> BoostPolygon;
typedef bg::model::multi_polygon<BoostPolygon> MultiPolygon;
typedef bg::model::segment<BoostPoint> BoostSegment;
typedef std::pair<bg::model::box<BoostPoint>, int> rtree_entry;

struct MotionPoint;
struct SimulateStatus;
class Path2D;
class Point2DGPU;

namespace CNCSYS
{
	enum class EntityType
	{
		Point,
		Line,
		Circle,
		Ellipse,
		Polyline,
		Arc,
		Spline,
	};

	class Shader;
	class OCSGPU;
	class CanvasGPU;
	class SketchGPU;
	class EntRingConnection;
	class EntGroup;

	class EntityVGPU
	{
	public:
		EntityVGPU() { id = EntityVGPU::counter++; }
		EntityVGPU(EntityVGPU* other) {
			id = EntityVGPU::counter++;
			this->bbox = other->bbox;
			this->area = other->area;
			this->boostPath = other->boostPath;
			this->pathLength = other->pathLength;
			this->direction = other->direction;
			this->modelMatrixStash = worldModelMatrix;
			this->color = other->color;
			this->isVisible = true;
			this->isSelected = false;
			this->isHover = false;
			this->vao = 0;
			this->vbo = 0;
		}
		EntityVGPU* Clone();
		void Copy(EntityVGPU* other);
		void SetHighLight(bool flag);
		void SetHoldCanvas(CanvasGPU* canvas);
		void ResetColor() { color = attribColor; }
		void ResetTransformation() {
			glm::mat4 worldModelMatrix = glm::mat4(1.0f); modelMatrixStash = worldModelMatrix;
			UpdatePaintData();
		}
		virtual EntityType GetType() const = 0;
		virtual void UpdatePaintData() = 0;
		virtual glm::vec3 GetStart() = 0;
		virtual glm::vec3 GetEnd() = 0;
		virtual void Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode) = 0;
		virtual void Move(const glm::vec3& offset) = 0;
		virtual void MoveTo(const glm::vec3& pos) = 0;
		virtual void Rotate(const glm::vec3& center, float angle) = 0;
		virtual void Scale(const glm::vec3& scalar, const glm::vec3& center) = 0;
		virtual void Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2) = 0;
		virtual void SetStartPoint(int index) = 0;
		virtual void Reverse() = 0;
		virtual glm::vec3 Evaluate(float t) = 0;
		virtual glm::vec3 Derivative(float t) = 0;
		virtual glm::vec3 Tangent(float t) { return glm::normalize(this->Derivative(t)); };
		virtual glm::vec3 Normal(float t) { glm::vec3 tan = this->Tangent(t); return glm::vec3(-tan.y, tan.x, 0.0f); };
		virtual float Curvature(float t) = 0;
		virtual float CurvatureRadius(float t) = 0;
		virtual std::vector<glm::vec3> GetTransformedNodes() = 0;
		virtual glm::vec3 GetTransformedCentroid() { return worldModelMatrix * glm::vec4(this->centroid, 1.0f); }
		virtual std::vector<glm::vec3> SplitToSection(float precision) = 0;
		//带G00移动到起始点
		virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) = 0;
		////不带G00移动到起始点
		virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) = 0;
		virtual QString Description() = 0;
		virtual EntityVGPU* Offset(float value, int precision);

		virtual ~EntityVGPU()
		{
			EntityVGPU::counter--;
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			if (vao != 0)
			{
				glDeleteVertexArrays(1, &vao);
				vao = 0;
			}
			if (vbo != 0)
			{
				glDeleteBuffers(1, &vbo);
				vbo = 0;
			}
			boostPath.clear();
		}

	protected:
		GLuint vao = 0;
		GLuint vbo = 0;
	public:
		static uint32_t counter;
		int id;
		AABB bbox;
		//-1表示中心点 -2表示当前无编辑点
		int editPointIndex = -2;
		double area;
		BoostLineString boostPath;
		float pathLength = 0.0f;

		GeomDirection direction;
		glm::mat4 worldModelMatrix = glm::mat4(1.0f);
		//暂存原始状态下的模型矩阵
		glm::mat4 modelMatrixStash = worldModelMatrix;
		glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		glm::vec4 attribColor = color;
		glm::vec3 centroid;
		bool isVisible = true;
		bool isSelected = false;
		bool isHover = false;
		bool openHighlight = false;
		bool createGCode = true;

		std::pair<GLint, GLint> highlightSection = { -1,-1 };
		//插值点索引范围
		std::pair<int, int> indexRange;
		CanvasGPU* canvasHandle;
		EntRingConnection* ringParent = nullptr;
		//位于哪个ring下
		uint32_t ringId;
	};

	class Ellipse2DGPU : public EntityVGPU {
	public:
		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar& center;
			ar& a;
			ar& b;
			ar& c;
			ar& worldModelMatrix;
		}

		Ellipse2DGPU();
		Ellipse2DGPU(Ellipse2DGPU* other);
		Ellipse2DGPU(glm::vec3 center, float radiusX, float radiusY);
		~Ellipse2DGPU();
		void Copy(Ellipse2DGPU* other);
		virtual void UpdatePaintData() override;
		virtual EntityType GetType() const override { return EntityType::Ellipse; }
		virtual glm::vec3 GetStart() override;
		virtual glm::vec3 GetEnd() override;
		virtual void Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode) override;
		virtual void Move(const glm::vec3& offset) override;
		virtual void MoveTo(const glm::vec3& pos) override;
		virtual void Rotate(const glm::vec3& center, float angle) override;
		virtual void Scale(const glm::vec3& scalar, const glm::vec3& center) override;
		virtual void Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2) override;
		virtual void SetStartPoint(int index) override;
		virtual void Reverse() override;
		virtual glm::vec3 Evaluate(float t) override;
		virtual glm::vec3 Derivative(float t) override;
		virtual float Curvature(float t) override;
		virtual float CurvatureRadius(float t) override;
		virtual std::vector<glm::vec3> GetTransformedNodes();
		virtual std::vector<glm::vec3> SplitToSection(float precision) override;
		virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
		virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
		virtual QString Description() override;
		void SetParameter(const glm::vec3& center, float a, float b);
		void GenerateEllipseSamplePoints(glm::vec3 center, float radiusX, float radiusY, int stepAngle, std::vector<glm::vec3>& samples);
	public:
		std::vector<glm::vec3> ellipseSamples;
		glm::vec3 center;
		float a, b, c;
	};

	struct EntityPoint
	{
		EntityVGPU* entity;
		int endPointIndex;
	};

	class EntRingConnection
	{
	public:
		EntRingConnection(const std::vector<EntityVGPU*>& _conponents);
		~EntRingConnection();

		EntRingConnection& operator=(const EntRingConnection& other)
		{
			if (this != &other)
			{
				conponents = other.conponents;
				bbox = other.bbox;
			}
		}

		void SetStartPoint(EntityVGPU* targetEntity, int index);
		void SetEndPoint(EntityVGPU* targetEntity, int index);
		std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr);
		glm::vec3 StartPoint() { return startPoint; }
		glm::vec3 EndPoint() { return endPoint; }
		void Move(const glm::vec3& offset);
		void ResetTransformation();

		void SelectAll();
		void UnSelectAll();
		void Reverse();
		void EraseEntity(EntityVGPU* e, SketchGPU* sketch);
		void AddEntity(EntityVGPU* e, SketchGPU* sketch);
	public:
		static uint32_t counter;

	public:
		std::vector<EntityVGPU*> conponents;
		Point2DGPU* centroidPoint = nullptr;
		AABB bbox;
		glm::vec3 startPoint;
		glm::vec3 endPoint;
		std::vector<glm::vec3> contour;
		double area = 0.0f;
		EntGroup* groupParent = nullptr;
		int processOrder = 0;
		GeomDirection direction;

		std::pair<EntityPoint, EntityPoint> processBoundary;
		int ringId;
	};

	class EntGroup
	{
	public:
		EntGroup();
		EntGroup(const std::vector<EntRingConnection*>& _rings);
		~EntGroup();
		void AddRingConnection(EntRingConnection* ring);
		void EraseRingConnection(EntRingConnection* ring, SketchGPU* sketch);
		void Move(const glm::vec3& offset);
		void ResetTransformation();
		glm::vec3 GetProcessStartPoint();
	public:
		std::vector<EntRingConnection*> rings;
		AABB bbox;
		glm::mat4 transform;
		int processOrder = 0;
	};

	class EntityVCPU : public QGraphicsItem
	{
	public:
		virtual EntityType GetType() const = 0;
		virtual std::string ToNcInstruction() = 0;

	protected:
		static long counter;
	public:
		AABB bbox;
		std::vector<float> jointPoints;
		float jointLength;
		double area;
		BoostPolygon boostPath;

		QColor color = Qt::green;
		glm::vec3 centroid;
		bool isSelected;
	};

	class Point2DCPU : public EntityVCPU
	{
	public:
		Point2DCPU();
		Point2DCPU(const glm::vec3& point);
		~Point2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Point; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		void SetParameter(const glm::vec3& pos);
		virtual std::string ToNcInstruction() override;
	private:
		glm::vec3 point;
	};

	class Line2DCPU : public EntityVCPU
	{
	public:
		Line2DCPU();
		Line2DCPU(const glm::vec3& start, const glm::vec3& end);
		~Line2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Line; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		void SetParameter(const glm::vec3& start, const glm::vec3& end);
		virtual std::string ToNcInstruction() override;
	private:
		glm::vec3 start;
		glm::vec3 end;
	};

	class Arc2DCPU : public EntityVCPU
	{
	public:
		Arc2DCPU();
		Arc2DCPU(const glm::vec3& center, float radius, float startAngle, float endAngle);
		~Arc2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Arc; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		void SetParameter(const glm::vec3& center, float startAngle, float endAngle, float radius);
		virtual std::string ToNcInstruction() override;

		void GenerateArcSamples(float startAngle, float endAngle, const glm::vec3& center, std::vector<glm::vec3>& samples);
	private:
		glm::vec3 start;
		glm::vec3 end;
		glm::vec3 center;
		float radius;
		float startAngle;
		float endAngle;
		std::vector<glm::vec3> arcSamples;
	};

	class Circle2DCPU : public EntityVCPU
	{
	public:
		Circle2DCPU();
		Circle2DCPU(const glm::vec3& center, float radius);
		~Circle2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Circle; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		virtual std::string ToNcInstruction() override;
		void SetParameter(const glm::vec3& center, float r);
		void GenerateCircleSamplePoints(const glm::vec3& center, float radius, int stepAngle, std::vector<glm::vec3>& samples);

	private:
		std::vector<glm::vec3> circleSamples;
		glm::vec3 center;
		float radius;
	};

	class Ellipse2DCPU : public EntityVCPU
	{
	public:
		Ellipse2DCPU();
		Ellipse2DCPU(const glm::vec3& center, float radiusX, float radiusY);
		~Ellipse2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Ellipse; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		virtual std::string ToNcInstruction() override;
		void SetParameter(const glm::vec3& center, float a, float b);

		void GenerateEllipseSamplePoints(glm::vec3 center, float radiusX, float radiusY, int stepAngle, std::vector<glm::vec3>& samples);
	private:
		std::vector<glm::vec3> ellipseSamples;
		glm::vec3 center;
		float a, b, c;
	};

	class Polyline2DCPU : public EntityVCPU
	{
	public:
		Polyline2DCPU();
		Polyline2DCPU(const std::vector<glm::vec3>& points, bool isClosed);
		~Polyline2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Polyline; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		void SetParameter(const std::vector<glm::vec3>& nodes, bool isClosed);
		virtual std::string ToNcInstruction() override;
	private:
		std::vector<glm::vec3> nodes;
		std::vector<float> bulges;
		bool isClosed;
	};

	class Spline2DCPU : public EntityVCPU
	{
	public:
		Spline2DCPU();
		Spline2DCPU(const std::vector<glm::vec3>& controlPoints, const std::vector<float> knots, bool isPassTrough);
		~Spline2DCPU();
		QRectF boundingRect(void) const override { return QRectF(QPointF(bbox.min.x, bbox.min.y), QSize(bbox.XRange(), bbox.YRange())); }
		virtual EntityType GetType() const override { return EntityType::Spline; }
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		void SetParameter(const std::vector<glm::vec3>& controlpoints, const std::vector<float>& knots, bool isPassthrough);
		virtual std::string ToNcInstruction() override;

		void GenerateSplineSamplePoints(const std::vector<glm::vec3>& controlPoints, std::vector<glm::vec3>& samples);
	private:
		std::vector<glm::vec3> controlPoints;
		std::vector<glm::vec3> splineSamples;
		std::vector<float> knots;
		bool isPassthrough;
	};

	inline std::ostream& operator<<(std::ostream& os, const BoostPoint& vec)
	{
		os << "(" << vec.x() << ", " << vec.y() << ")";
		return os;
	}
}