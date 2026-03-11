#include "Graphics/Arc2D.h"
#include "Common/ProgressInfo.h"
#include "Controls/GCodeController.h"
#include "Graphics/Sketch.h"
#include "UI/GCodeEditor.h"

Arc2DGPU::Arc2DGPU()
{
}

Arc2DGPU::Arc2DGPU(Arc2DGPU* other) : EntityVGPU(other)
{
	this->SetParameter(other->center, other->radius, other->startAngle, other->endAngle);
}

Arc2DGPU::Arc2DGPU(glm::vec3 center, float radius, float startAngle, float endAngle) : center(center), radius(radius), startAngle(startAngle), endAngle(endAngle)
{
	arcSamples.clear();

	area = abs((startAngle - endAngle) * deg2Rad / (PI * radius));
	this->center = center;
	this->startAngle = startAngle;
	this->endAngle = endAngle;
	this->start = glm::vec3(center.x + radius * (float)cos(startAngle * deg2Rad), center.y + radius * (float)sin(startAngle * deg2Rad), 0.0f);
	this->end = glm::vec3(center.x + radius * (float)cos(endAngle * deg2Rad), center.y + radius * (float)sin(endAngle * deg2Rad), 0.0f);

	bbox = AABB(start, end);

	GenerateArcSamples(this->startAngle, this->endAngle, center, arcSamples);

	centroid = glm::vec3(0.0);
	for (const glm::vec3& vec : arcSamples)
	{
		bg::append(boostPath, BoostPoint(vec.x, vec.y));
		bbox.Union(vec);
		centroid += vec;
	}

	pathLength = 2 * PI * radius * (endAngle - startAngle) / 360.0f;

	centroid /= arcSamples.size();
	direction = MathUtils::GetDirection(this->centroid, this->start, this->end);
	UpdatePaintData();
}
Arc2DGPU::~Arc2DGPU()
{

}

void Arc2DGPU::Copy(Arc2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->center, other->startAngle, other->endAngle, other->radius);
}

void Arc2DGPU::UpdatePaintData()
{
	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, arcSamples.size() * sizeof(glm::vec3), arcSamples.data(), GL_STATIC_DRAW);
	}

	int i = 0;
	for (int i = 0; i < arcSamples.size(); i++)
	{
		glm::vec3 transformed = worldModelMatrix * glm::vec4(arcSamples[i], 1.0f);
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

void Arc2DGPU::GenerateArcSamples(float startAngle, float endAngle, const glm::vec3& center, std::vector<glm::vec3>& samples)
{
	if (samples.size())
		samples.clear();

	if (endAngle < startAngle)
	{
		endAngle = endAngle + 360.0f;
	}
	float step = std::min(1.0f, (endAngle - startAngle) / 10.0f);

	samples.reserve((int)((endAngle - startAngle) / step) + 2);
	for (float angle = startAngle; angle <= endAngle; angle += step)
	{
		float x = (float)(radius * cos(angle * deg2Rad));
		float y = (float)(radius * sin(angle * deg2Rad));

		samples.push_back(glm::vec3(center.x + x, center.y + y, 0.0f));
		bbox.Union(glm::vec3(x, y, 0.0f));
	}

	float x = (float)(radius * cos(endAngle * deg2Rad));
	float y = (float)(radius * sin(endAngle * deg2Rad));
	samples.push_back(glm::vec3(center.x + x, center.y + y, 0.0f));
	bbox.Union(glm::vec3(x, y, 0.0f));
	indexRange = { 0,samples.size() - 1 };
}

void Arc2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glBufferData(GL_ARRAY_BUFFER, arcSamples.size() * sizeof(glm::vec3), arcSamples.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);
		}

		shader->use();
		shader->setVec4("PaintColor", color);
		shader->setMat4("model", worldModelMatrix);
		if (mode != RenderMode::Point)
		{
			if (isSelected)
			{
				shader->setFloat("minVisibleLength", 1.0f);
				//shader->setInt("toatalVertexCount", 2);
				//shader->setInt("dashCount", 50);
			}
			glLineWidth(g_lineWidth);
			glBindVertexArray(vao);
			glDrawArrays(GL_LINE_STRIP, 0, arcSamples.size());

			if (openHighlight)
			{
				glLineWidth(4.0f);
				shader->setVec4("PaintColor", g_highlightColor);
				glDrawArrays(GL_LINE_STRIP, 0, arcSamples.size());
				glLineWidth(2);
			}
		}
		if (g_canvasInstance->showInnerPoint)
		{
			g_pointShader->use();
			g_pointShader->setMat4("model", worldModelMatrix);
			g_pointShader->setVec4("PaintColor", g_whiteColor);
			glDrawArrays(GL_POINTS, 0, arcSamples.size());
		}
	}
}
glm::vec3 Arc2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(this->arcSamples[0], 1.0f);
}
glm::vec3 Arc2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(this->arcSamples[arcSamples.size() - 1], 1.0f);
}
void Arc2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}
void Arc2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->center, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}
void Arc2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;
}
void Arc2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
	glm::vec3 offsetToOrigin = -center;

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

	worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
}
void Arc2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{
	std::vector<glm::vec3> transformedNodes = GetTransformedNodes();

	for (int i = 0; i < transformedNodes.size(); i++)
	{
		arcSamples[i] = MathUtils::reflectPoint(transformedNodes[i], linePt1, linePt2);
	}
	this->centroid = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->centroid, 1.0f), linePt1, linePt2);
	this->start = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->start, 1.0f), linePt1, linePt2);
	this->end = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->end, 1.0f), linePt1, linePt2);
}

void Arc2DGPU::SetStartPoint(int index)
{
	if (index != 0)
	{
		std::vector<glm::vec3> temp;
		for (int i = index; i < arcSamples.size(); i++)
		{
			temp.push_back(arcSamples[i]);
		}
		for (int i = 0; i < index; i++)
		{
			temp.push_back(arcSamples[i]);
		}
		arcSamples = temp;
	}
}

void Arc2DGPU::Reverse()
{
	std::swap(this->start, this->end);
	//std::swap(this->startAngle, this->endAngle);
	std::reverse(this->arcSamples.begin(), this->arcSamples.end());

	if (direction == GeomDirection::CW)
	{
		direction = GeomDirection::CCW;
	}
	else
	{
		direction = GeomDirection::CW;
	}

	UpdatePaintData();
}

glm::vec3 Arc2DGPU::Evaluate(float t)
{
	float angle = startAngle + t * (endAngle - startAngle);
	glm::vec3 evaluated = glm::vec3(center.x + radius * (float)cos(angle * deg2Rad), center.y + radius * (float)sin(angle * deg2Rad), 0.0f);
	return this->worldModelMatrix * glm::vec4(evaluated, 1.0f);
}

glm::vec3 Arc2DGPU::Derivative(float t)
{
	float angle = startAngle + t * (endAngle - startAngle);
	glm::vec3 drv = glm::vec3(-radius * (float)sin(angle * deg2Rad), radius * (float)cos(angle * deg2Rad), 0.0f);
	return this->worldModelMatrix * glm::vec4(drv, 1.0f);
}

float Arc2DGPU::Curvature(float t)
{
	return 1.0f / radius;
}

float Arc2DGPU::CurvatureRadius(float t)
{
	return radius;
}

void Arc2DGPU::SetParameter(const glm::vec3& center, float startAngle, float endAngle, float radius)
{
	arcSamples.clear();

	area = abs((startAngle - endAngle) * deg2Rad / (PI * radius));
	this->center = center;
	this->start = glm::vec3(center.x + radius * (float)cos(startAngle * deg2Rad), center.y + radius * (float)sin(startAngle * deg2Rad), 0.0f);
	this->end = glm::vec3(center.x + radius * (float)cos(endAngle * deg2Rad), center.y + radius * (float)sin(endAngle * deg2Rad), 0.0f);
	this->startAngle = startAngle;
	this->endAngle = endAngle;
	this->radius = radius;

	bbox = AABB(start, end);

	GenerateArcSamples(this->startAngle, this->endAngle, center, arcSamples);

	centroid = glm::vec3(0.0f);
	boostPath.clear();
	for (const glm::vec3& p : arcSamples)
	{
		bg::append(boostPath, BoostPoint(p.x, p.y));
		centroid += p;
		bbox.Union(p);
	}
	centroid /= arcSamples.size();
	pathLength = 2 * PI * radius * (endAngle - startAngle) / 360.0f;

	direction = MathUtils::GetDirection(centroid, start, end);
	UpdatePaintData();
}

std::vector<glm::vec3> Arc2DGPU::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	if (arcSamples.empty())
	{
		GenerateArcSamples(startAngle, endAngle, center, arcSamples);
	}

	for (auto& pt : arcSamples)
	{
		res.push_back(this->worldModelMatrix * glm::vec4(pt, 1.0f));
	}
	return res;
}

std::vector<glm::vec3> Arc2DGPU::SplitToSection(float precision)
{
	float step = precision / (radius * abs(endAngle - startAngle) * deg2Rad);
	float t = 0;
	std::vector<glm::vec3> res;

	while (t < 1)
	{
		res.push_back(Evaluate(t));
		t += step;
	}

	res.push_back(worldModelMatrix * glm::vec4(end, 1.0f));

	return res;
}

std::string Arc2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 start = transformedMatrix * glm::vec4(this->start, 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(this->end, 1.0f);

		glm::vec3 center = transformedMatrix * glm::vec4(this->center, 1.0f);

		float I = (center - start).x;
		float J = (center - start).y;
		char buffer[256];
		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}

		if (direction == GeomDirection::CCW)
		{
			std::sprintf(buffer, "N%03d G03 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, arcSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		else
		{
			std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, arcSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		Mstatus->toolPos = end;
	}
	return s;
}

std::string Arc2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string section = "";
	if (createGCode)
	{
		char buffer[256];
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 start = transformedMatrix * glm::vec4(this->start, 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(this->end, 1.0f);

		glm::vec3 center = transformedMatrix * glm::vec4(this->center, 1.0f);

		float I = (center - start).x;
		float J = (center - start).y;

		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			sprintf(buffer,"N%03d G00 X%f Y%f\n",Mstatus->ncstep++,start.x,start.y);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			Mstatus->totalPath += glm::distance(start, Mstatus->toolPos);
			Mstatus->idlePath += glm::distance(start, Mstatus->toolPos);
		}

		if (direction == GeomDirection::CCW)
		{
			std::sprintf(buffer, "N%03d G03 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, arcSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		else
		{
			std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, arcSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		Mstatus->totalPath += pathLength;
		Mstatus->toolPos = end;
	}
	return section;
}

QString Arc2DGPU::Description()
{
	QString strDirection;
	if (direction == GeomDirection::CW)
	{
		strDirection = ("顺时针");
	}
	else if (direction == GeomDirection::CCW)
	{
		strDirection = ("逆时针");
	}

	QString PointDesc;
	if (editPointIndex > 0)
	{
		glm::vec3 editPointPos = worldModelMatrix * glm::vec4(arcSamples[editPointIndex], 1.0f);

		PointDesc = QString(",编辑点(%1):(%2,%3)").arg(editPointIndex).arg(editPointPos.x).arg(editPointPos.y);
	}

	glm::vec3 transformedCenter = worldModelMatrix * glm::vec4(center, 1.0f);

	return QString("圆弧,圆心(%1,%2),半径:%3,%4,角度(%5,%6) 图形总长:%7 %8")
		.arg(transformedCenter.x).arg(transformedCenter.y).arg(radius).arg(strDirection).arg(startAngle).arg(endAngle).arg(pathLength).arg(PointDesc);
}