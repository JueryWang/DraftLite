#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include "Common/MathUtils.h"
#include "Graphics/OCS.h"
#include "Controls/GCodeController.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Graphics/SelectionBox.h"
#include "UI/GCodeEditor.h"
#include "Algorithm/RingDetector.h"
#include "clipper2/clipper.h"
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <QMessageBox>

namespace cl = Clipper2Lib;

namespace CNCSYS
{
	uint32_t EntityVGPU::counter = 0;
	uint32_t EntRingConnection::counter = 0;

	EntityVGPU* EntityVGPU::Clone()
	{
		switch (this->GetType())
		{
		case EntityType::Point:
			return new Point2DGPU(static_cast<Point2DGPU*>(this));
		case EntityType::Line:
			return new Line2DGPU(static_cast<Line2DGPU*>(this));
		case EntityType::Circle:
			return new Circle2DGPU(static_cast<Circle2DGPU*>(this));
		case EntityType::Arc:
			return new Arc2DGPU(static_cast<Arc2DGPU*>(this));
		case EntityType::Polyline:
			return new Polyline2DGPU(static_cast<Polyline2DGPU*>(this));
		case EntityType::Ellipse:
			return new Ellipse2DGPU(static_cast<Ellipse2DGPU*>(this));
		case EntityType::Spline:
			return new Spline2DGPU(static_cast<Spline2DGPU*>(this));
		default:
			return nullptr;
		}
	}

	void EntityVGPU::Copy(EntityVGPU* other)
	{
		this->bbox = other->bbox;
		this->area = other->area;
		this->boostPath = other->boostPath;
		this->pathLength = other->pathLength;
		this->direction = other->direction;
		this->worldModelMatrix = other->worldModelMatrix;
		this->modelMatrixStash = worldModelMatrix;
		this->color = other->color;
		this->isVisible = other->isVisible;
		this->isSelected = false;
		this->isHover = false;
		this->ringId = other->ringId;
		this->createGCode = other->createGCode;
		if (!this->createGCode)
		{
			this->attribColor = g_darkGreen;
			this->ResetColor();
		}
	}

	void EntityVGPU::SetHighLight(bool flag)
	{
		openHighlight = flag;
		if (!openHighlight)
		{
			highlightSection = { -1,-1 };
		}
	}

	void EntityVGPU::SetHoldCanvas(CanvasGPU* canvas)
	{
		canvasHandle = canvas;
	}

	EntityVGPU* EntityVGPU::Offset(float value, int precision)
	{
		cl::Path64 subject;

		for (const BoostPoint& p : this->boostPath)
		{
			subject.emplace_back(bg::get<0>(p) * precision, bg::get<1>(p) * precision);
		}

		cl::Paths64 solution;
		solution.push_back(subject);

		cl::ClipperOffset offsetter;
		offsetter.AddPaths(solution, cl::JoinType::Round, cl::EndType::Polygon);

		offsetter.Execute(value * precision, solution);

		std::vector<glm::vec3> offsetNodes;

		for (const auto& path : solution) {
			for (const auto& pt : path) {
				offsetNodes.push_back({ (float)pt.x / 1000.0,(float)pt.y / 1000.0,0.0f });
			}
		}

		if (offsetNodes.size() == 0)
		{
			//QMessageBox::warning(nullptr, "偏移失败", "偏移失败,可能是偏置参数设置过大");
			return nullptr;
		}

		if (offsetNodes[0] != offsetNodes[offsetNodes.size() - 1])
			offsetNodes.push_back(offsetNodes[0]);

		if (offsetNodes.size() > 1)
		{
			Polyline2DGPU* poly = new Polyline2DGPU(offsetNodes, true);
			std::vector<float> bulges(offsetNodes.size() - 1, 0);
			poly->SetParameter(offsetNodes, true, bulges);
			return poly;
		}

		return nullptr;
	}

	Ellipse2DGPU::Ellipse2DGPU()
	{

	}

	Ellipse2DGPU::Ellipse2DGPU(Ellipse2DGPU* other) : EntityVGPU(other)
	{
		this->SetParameter(other->center, other->a, other->b);
	}

	Ellipse2DGPU::Ellipse2DGPU(glm::vec3 center, float radiusX, float radiusY) : center(center), a(radiusX), b(radiusY)
	{
		bbox = AABB(glm::vec3(center.x - radiusX, center.y - radiusY, 0.0), glm::vec3(center.x + radiusX, center.y + radiusX, 0.0f));

		GenerateEllipseSamplePoints(center, radiusX, radiusY, 5, ellipseSamples);

		boostPath.clear();
		centroid = glm::vec3(0.0f);
		for (auto& vec : ellipseSamples)
		{
			bg::append(boostPath, BoostPoint(vec.x, vec.y));
			centroid += vec;
		}
		centroid /= ellipseSamples.size();

		pathLength = 0;
		int size = ellipseSamples.size();
		for (int i = 0; i < size - 1; i++)
		{
			pathLength += glm::distance(ellipseSamples[i], ellipseSamples[i + 1]);
		}
		pathLength += glm::distance(ellipseSamples[size - 1], ellipseSamples[0]);
		direction = MathUtils::GetDirection(this->centroid, ellipseSamples[0], ellipseSamples[1]);
		UpdatePaintData();
	}

	Ellipse2DGPU::~Ellipse2DGPU()
	{

	}
	void Ellipse2DGPU::Copy(Ellipse2DGPU* other)
	{
		this->EntityVGPU::Copy(other);
		this->SetParameter(other->center, other->a, other->b);
	}

	void Ellipse2DGPU::UpdatePaintData()
	{
		if (vao > 0 && vbo > 0)
		{
			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, ellipseSamples.size() * sizeof(glm::vec3), ellipseSamples.data(), GL_STATIC_DRAW);
		}

		int i = 0;
		for (int i = 0; i < ellipseSamples.size(); i++)
		{
			glm::vec3 transformed = worldModelMatrix * glm::vec4(ellipseSamples[i], 1.0f);
			bg::set<0>(boostPath[i], transformed.x);
			bg::set<1>(boostPath[i], transformed.y);
		}

		BoostBox boostBox;
		bg::envelope(boostPath, boostBox);
		BoostPoint min = boostBox.min_corner();
		BoostPoint max = boostBox.max_corner();
		bbox = AABB(glm::vec3(bg::get<0>(min), bg::get<1>(min), 0.0f), glm::vec3(bg::get<0>(max), bg::get<1>(max), 0.0f));
		this->modelMatrixStash = this->worldModelMatrix;
	}
	glm::vec3 Ellipse2DGPU::GetStart()
	{
		return glm::vec3();
	}
	glm::vec3 Ellipse2DGPU::GetEnd()
	{
		return glm::vec3();
	}
	void Ellipse2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
	{
		if (isVisible)
		{
			if (vao == 0)
			{
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);

				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);

				glBufferData(GL_ARRAY_BUFFER, ellipseSamples.size() * sizeof(glm::vec3), ellipseSamples.data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0);
				glEnableVertexAttribArray(0);
			}
			shader->use();
			shader->setMat4("model", worldModelMatrix);
			shader->setVec4("PaintColor", color);
			if (mode != RenderMode::Point)
			{
				if (isSelected)
				{
					shader->setFloat("minVisibleLength", 1.0f);
					//shader->setInt("toatalVertexCount", 2);
					//shader->setInt("dashCount", 50);
				}
				glBindVertexArray(vao);
				glDrawArrays(GL_LINE_STRIP, 0, ellipseSamples.size());
			}
			else
			{
				glDrawArrays(GL_POINTS, 0, ellipseSamples.size());
			}
		}
		//shader->setVec4("PaintColor", g_whiteColor);
		//glDrawArrays(GL_POINTS, 0, 1);
	}
	void Ellipse2DGPU::Move(const glm::vec3& offset)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
		worldModelMatrix = translation * worldModelMatrix;
	}
	void Ellipse2DGPU::MoveTo(const glm::vec3& pos)
	{
		glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->center, 1.0f);
		glm::vec3 offset = pos - worldCentroid;
		Move(offset);
	}
	void Ellipse2DGPU::Rotate(const glm::vec3& center, float angle)
	{
		float rotateDeg = glm::radians(angle);
		glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
		glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

		glm::mat4 transform = translateBack * rotation * translateToOrigin;

		worldModelMatrix = transform * worldModelMatrix;
	}
	void Ellipse2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
	{
		glm::vec3 offsetToOrigin = -center;

		glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
		glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
		glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

		worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
	}
	void Ellipse2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
	{
		std::vector<glm::vec3> transformedNodes = GetTransformedNodes();

		for (int i = 0; i < transformedNodes.size(); i++)
		{
			ellipseSamples[i] = MathUtils::reflectPoint(transformedNodes[i], linePt1, linePt2);
		}
		this->centroid = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->centroid, 1.0f), linePt1, linePt2);
		this->center = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->center, 1.0f), linePt1, linePt2);
	}

	void Ellipse2DGPU::SetStartPoint(int index)
	{
		if (index != 0)
		{
			std::vector<glm::vec3> temp;
			for (int i = index; i < ellipseSamples.size(); i++)
			{
				temp.push_back(ellipseSamples[i]);
			}
			for (int i = 0; i < index; i++)
			{
				temp.push_back(ellipseSamples[i]);
			}
			ellipseSamples = temp;
		}
	}

	void Ellipse2DGPU::Reverse()
	{

	}

	glm::vec3 Ellipse2DGPU::Evaluate(float t)
	{
		return glm::vec3();
	}

	glm::vec3 Ellipse2DGPU::Derivative(float t)
	{
		return glm::vec3();
	}

	float Ellipse2DGPU::Curvature(float t)
	{
		return 0.0f;
	}

	float Ellipse2DGPU::CurvatureRadius(float t)
	{
		return 0.0f;
	}

	std::vector<glm::vec3> Ellipse2DGPU::SplitToSection(float precision)
	{
		return std::vector<glm::vec3>();
	}

	void Ellipse2DGPU::SetParameter(const glm::vec3& center, float a, float b)
	{
	}

	std::vector<glm::vec3> Ellipse2DGPU::GetTransformedNodes()
	{
		std::vector<glm::vec3> res;
		for (auto& pt : ellipseSamples)
		{
			res.push_back(this->worldModelMatrix * glm::vec4(pt, 1.0f));
		}
		return res;
	}

	std::string Ellipse2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
	{
		std::string s;
		return s;
	}

	std::string Ellipse2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
	{
		return std::string();
	}

	QString Ellipse2DGPU::Description()
	{
		return QString();
	}

	void Ellipse2DGPU::GenerateEllipseSamplePoints(glm::vec3 center, float radiusX, float radiusY, int stepAngle, std::vector<glm::vec3>& samples)
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

	EntRingConnection::EntRingConnection(const std::vector<EntityVGPU*>& _conponents) : conponents(_conponents)
	{
		glm::vec3 centroid = glm::vec3(0.0f);
		glm::vec3 start = conponents[0]->GetStart();

		bbox = AABB(start, start);
		
		int i = 0;
		for (EntityVGPU* ent : conponents)
		{
			std::vector<glm::vec3> nodes = ent->GetTransformedNodes();
			for (auto& p : nodes)
			{
				bbox.Union(p);
			}
			centroid += ent->GetTransformedCentroid();
			contour.insert(contour.end(), nodes.begin(), nodes.end());
			ent->ringId = EntRingConnection::counter;
			centroid += ent->centroid;
		}

		area = MathUtils::ComputeArea(contour);
		centroid /= conponents.size();

		startPoint = conponents[0]->GetTransformedNodes()[0];
		endPoint = conponents.back()->GetTransformedNodes()[0];
		glm::vec3 midPoint = conponents[conponents.size() / 2]->GetTransformedNodes()[0];
		direction = MathUtils::GetDirection(midPoint, startPoint, endPoint);

		ringId = EntRingConnection::counter++;
	}

	EntRingConnection::~EntRingConnection()
	{
		conponents.clear();
		contour.clear();
	}

	EntityVGPU* EntRingConnection::ToPolyline()
	{
		Polyline2DGPU* polyline = new Polyline2DGPU();
		std::vector<glm::vec3> polylineNodes;
		for (EntityVGPU* ent : conponents)
		{
			auto nodes = ent->GetTransformedNodes();
			std::copy(nodes.begin(), nodes.end(),std::back_inserter(polylineNodes));
		}
		polyline->SetParameter(polylineNodes,false);

		return polyline;
	}

	void EntRingConnection::RepairStart()
	{
		int res = 0;
		int falseTotal = 0;
		int firstTruePos = -1;
		int secondTruePos = -1;

		for (int i = 0; i < this->conponents.size(); i++)
		{
			conponents[i]->ringParent = this;
			if (this->conponents[i]->createGCode == false)
			{
				falseTotal++;
			}
		}

		int falseCount = 0;
		for (int i = 0; i < this->conponents.size(); i++)
		{
			if (conponents[i]->createGCode == false)
			{
				falseCount++;
			}
			else
			{
				if (falseCount <= falseTotal && (firstTruePos == -1))
				{
					firstTruePos = i;
				}
				else if(secondTruePos == -1)
				{
					secondTruePos = i;
					break;
				}
			}
		}

		if (conponents[0]->createGCode == false)
		{
			res = firstTruePos;
		}
		else
		{
			res = secondTruePos;
		}

		if (res >= 0)
		{
			this->SetStartPoint(this->conponents[res], 0);
		}
	}

	void EntRingConnection::SetStartPoint(EntityVGPU* targetEntity, int index)
	{
		if (index >= 0)
		{
			startPoint = targetEntity->GetTransformedNodes()[index];
		}
		auto find = std::find(conponents.begin(), conponents.end(), targetEntity);
		std::vector<EntityVGPU*> reordered;
		//起始点,该Entity计入起始
		int findIndex = find - conponents.begin();
		for (int i = findIndex; i < conponents.size(); i++)
		{
			reordered.push_back(conponents[i]);
		}
		for (int i = 0; i < findIndex; i++)
		{
			reordered.push_back(conponents[i]);
		}
		conponents = reordered;
		EntityPoint endPoint;
		endPoint.entity = targetEntity;
		endPoint.endPointIndex = index;
		//排序后Boundary的第一个默认为conponent第一个元素
		processBoundary.first = endPoint;
	}

	void EntRingConnection::SetEndPoint(EntityVGPU* targetEntity, int index)
	{
		if (index >= 0)
		{
			endPoint = targetEntity->GetTransformedNodes()[index];
		}
		EntityPoint endPoint;
		endPoint.entity = targetEntity;
		endPoint.endPointIndex = index;

		processBoundary.second = endPoint;
		//重置
		bool discard = false;
		//起始选择点落在终点,当前段不选
		if (processBoundary.first.endPointIndex == conponents[0]->indexRange.second)
		{
			conponents[0]->attribColor = g_darkGreen;
			conponents[0]->ResetColor();
			conponents[0]->createGCode = false;
		}
		for (int i = 0; i < conponents.size(); i++)
		{
			if (processBoundary.second.entity == conponents[i])
			{
				discard = true;
				if (processBoundary.second.endPointIndex != 0)
				{
					continue;
				}
			}
			if (discard)
			{
				conponents[i]->attribColor = g_darkGreen;
				conponents[i]->ResetColor();
				conponents[i]->createGCode = false;
			}
		}
	}

	std::string EntRingConnection::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
	{
		std::string s;
		if (conponents.size() > 0)
		{
			//char buffer[256];

			//glm::vec3 startPoint = StartPoint();
			//glm::vec3 leftBottom = bbox.getMin();
			//g_MScontext.XAxisStart = startPoint.x - leftBottom.x;
			//g_MScontext.YAxisStart = startPoint.y - leftBottom.y;
			//g_MScontext.ZAxisStart = startPoint.z - leftBottom.z;
			//g_MScontext.wcsAnchor = startPoint;

			//g_MScontext.objectRange = g_canvasInstance->GetOCSSystem()->objectRange;
			//g_MScontext.zoom = 1.0f;
			//g_MScontext.toolPos = glm::vec3(0.0f);
			//int step = 0;

			////开头写入M辅助码
			//char MBuffer[256];
			//std::sprintf(MBuffer, "N%03d M1 K%f L%f\n", g_MScontext.ncstep, g_MScontext.XAxisStart, g_MScontext.YAxisStart);
			//s += MBuffer;
			//g_MScontext.ncstep++;
			//GCodeRecord rec(std::string(MBuffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
			//GCodeController::GetController()->AddRecord(rec);

			//{
			//	std::sprintf(MBuffer, "N%03d M2 K%f\n", g_MScontext.ncstep, g_MScontext.ZAxisStart);
			//	s += MBuffer;
			//	g_MScontext.ncstep++;
			//	rec = GCodeRecord(std::string(MBuffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
			//	GCodeController::GetController()->AddRecord(rec);
			//}
			//
			////计算刀补偏置后起点
			//float toolDistance = g_MScontext.GetToolDistance();
			//float toolRadius = g_MScontext.GetToolRadius();

			//{
			//	//给定进刀位置
			//	if (direction == GeomDirection::CW)
			//	{
			//		std::sprintf(MBuffer, "N%03d G81 X%f Y%f\n", g_MScontext.ncstep, 1.5 * -toolRadius, 1.5 * -toolRadius);
			//	}
			//	else
			//	{
			//		std::sprintf(MBuffer, "N%03d G81 X%f Y%f\n", g_MScontext.ncstep, 1.5 * toolRadius, 1.5 * toolRadius);
			//	}
			//	rec = GCodeRecord(std::string(MBuffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
			//	GCodeController::GetController()->AddRecord(rec);
			//	g_MScontext.ncstep++;
			//	s += MBuffer;
			//}

			//{
			//	if (direction == GeomDirection::CW)
			//	{
			//		sprintf(MBuffer, "N%03d G41 D%f\n", g_MScontext.ncstep, toolRadius);
			//	}
			//	else
			//	{
			//		sprintf(MBuffer, "N%03d G42 D%f\n", g_MScontext.ncstep, toolRadius);
			//	}
			//	g_MScontext.ncstep++;
			//	s += MBuffer;
			//	rec = GCodeRecord(std::string(MBuffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
			//	GCodeController::GetController()->AddRecord(rec);
			//}

			//{
			//	std::sprintf(MBuffer, "N%03d G01 X0 Y0\n", g_MScontext.ncstep);
			//	g_MScontext.ncstep++;
			//	s += MBuffer;
			//	rec = GCodeRecord(std::string(MBuffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
			//	GCodeController::GetController()->AddRecord(rec);
			//}
			for (EntityVGPU* ent : conponents)
			{
				if (ent->createGCode)
				{
					s += ent->GenNcSection(Mstatus, createRecord, sketch);
				}
			}
		}

		return s;
	}

	void EntRingConnection::SelectAll()
	{
		for (EntityVGPU* e : conponents)
		{
			e->isSelected = true;
		}
	}
	void EntRingConnection::UnSelectAll()
	{
		for (EntityVGPU* e : conponents)
		{
			e->isSelected = false;
		}
	}

	void EntRingConnection::Reverse()
	{
		std::reverse(conponents.begin(), conponents.end());
		int i = 0;
		for (EntityVGPU* conponent : conponents)
		{
			conponent->highlightSection = { -1,-1 };
			conponent->SetHighLight(false);
			conponent->Reverse();
		}
		std::swap(startPoint, endPoint);
	}

	void EntRingConnection::Move(const glm::vec3& offset)
	{
		for (EntityVGPU* ent : conponents)
		{
			ent->Move(offset);
			ent->UpdatePaintData();
		}

		bbox.Translate(offset);
		startPoint += offset;
		endPoint += offset;
	}

	void EntRingConnection::ResetTransformation()
	{
		for (EntityVGPU* ent : conponents)
		{
			ent->ResetTransformation();
		}

		glm::vec3 centroid = glm::vec3(0.0f);
		glm::vec3 start = conponents[0]->GetStart();
		contour.clear();

		bbox = AABB(start, start);
		for (EntityVGPU* ent : conponents)
		{
			std::vector<glm::vec3> nodes = ent->GetTransformedNodes();
			for (auto& p : nodes)
			{
				bbox.Union(p);
			}

			centroid += ent->GetTransformedCentroid();
			contour.insert(contour.end(), nodes.begin(), nodes.end());
		}
		area = MathUtils::ComputeArea(contour);
		centroid /= conponents.size();

		startPoint = conponents[0]->GetTransformedNodes()[0];
		endPoint = conponents.back()->GetTransformedNodes().back();
	}

	void EntRingConnection::EraseEntity(EntityVGPU* e, SketchGPU* sketch)
	{
		auto find = std::find(conponents.begin(), conponents.end(), e);
		if (find != conponents.end())
		{
			conponents.erase(find);
			delete e;
		}
		if (conponents.size() == 0)
		{
			EntGroup* parent = this->groupParent;
			parent->EraseRingConnection(this, sketch);
		}
	}

	void EntRingConnection::AddEntity(EntityVGPU* e, SketchGPU* sketch)
	{
		
	}

	EntGroup::EntGroup()
	{

	}

	EntGroup::EntGroup(const std::vector<EntRingConnection*>& _rings) : rings(_rings)
	{
		bbox = rings[0]->bbox;
		int order = 0;
		for (EntRingConnection* ring : rings)
		{
			ring->processOrder = order++;
			bbox.Union(ring->bbox);
		}
	}

	void EntGroup::ResetTransformation()
	{
		rings[0]->ResetTransformation();
		bbox = rings[0]->bbox;
		for (int i = 1; i < rings.size(); i++)
		{
			bbox.Union(rings[i]->bbox);
		}
	}

	EntGroup::~EntGroup()
	{
		for (EntRingConnection* ring : rings)
		{
			delete ring;
		}
		rings.clear();
	}

	void EntGroup::AddRingConnection(EntRingConnection* ring)
	{
		if (rings.size() == 0)
		{
			bbox = ring->bbox;
		}
		else
		{
			bbox.Union(ring->bbox);
		}
		ring->processOrder = rings.size();
		rings.push_back(ring);
	}

	void EntGroup::EraseRingConnection(EntRingConnection* ring, SketchGPU* sketch)
	{
		auto find = std::find(rings.begin(), rings.end(), ring);
		if (find != rings.end())
		{
			rings.erase(find);
			delete ring;
		}
		if (rings.size() == 0)
		{
			auto findGroup = std::find(sketch->entityGroups.begin(), sketch->entityGroups.end(), this);
			if (findGroup != sketch->entityGroups.end())
			{
				sketch->entityGroups.erase(findGroup);
			}
			delete this;
		}
	}

	void EntGroup::Move(const glm::vec3& offset)
	{
		for (EntRingConnection* ring : rings)
		{
			ring->Move(offset);
		}
		bbox.Translate(offset);
	}

	glm::vec3 EntGroup::GetProcessStartPoint()
	{
		return rings[0]->startPoint;
	}
}

