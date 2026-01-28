#include "Graphics/Line2D.h"
#include "Common/ProgressInfo.h"
#include "Controls/GCodeController.h"
#include "UI/GCodeEditor.h"
#include "Graphics/Sketch.h"
#include "Path/Path.h"
#include "Common/MathUtils.h"

Line2DGPU::Line2DGPU()
{

}

Line2DGPU::Line2DGPU(Line2DGPU* other) : EntityVGPU(other)
{
	this->SetParameter(other->start, other->end);
}

Line2DGPU::Line2DGPU(glm::vec3 start, glm::vec3 end) : start(start), end(end)
{
	bbox = AABB(start, end);

	centroid = (start + end);
	centroid /= 2;

	bg::append(boostPath, BoostPoint(this->start.x, this->start.y));
	bg::append(boostPath, BoostPoint(this->end.x, this->end.y));
	pathLength = glm::distance(start, end);

	UpdatePaintData();
	direction = MathUtils::GetDirection(this->centroid, this->start, this->end);
	indexRange = { 0,1 };
}

Line2DGPU::~Line2DGPU()
{

}
void Line2DGPU::Copy(Line2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->start, other->end);
}

void Line2DGPU::UpdatePaintData()
{

	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glm::vec3 arrays[] = { this->start,this->end };
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), arrays, GL_STATIC_DRAW);
	}

	glm::vec3 transformedStart = worldModelMatrix * glm::vec4(this->start, 1.0f);
	glm::vec3 transformedEnd = worldModelMatrix * glm::vec4(this->end, 1.0f);

	bg::set<0>(boostPath[0], transformedStart.x);
	bg::set<1>(boostPath[0], transformedStart.y);
	bg::set<0>(boostPath[1], transformedEnd.x);
	bg::set<1>(boostPath[1], transformedEnd.y);

	BoostBox boostBox;
	bg::envelope(boostPath, boostBox);
	BoostPoint min = boostBox.min_corner();
	BoostPoint max = boostBox.max_corner();
	bbox = AABB(glm::vec3(bg::get<0>(min), bg::get<1>(min), 0.0f), glm::vec3(bg::get<0>(max), bg::get<1>(max), 0.0f));
	this->modelMatrixStash = this->worldModelMatrix;
}
void Line2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glm::vec3 vertices[] = { start,end };
			glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);

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
			}

			glLineWidth(g_lineWidth);
			glBindVertexArray(vao);
			glDrawArrays(GL_LINES, 0, 2);

			if (openHighlight)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glLineWidth(4.0f);
				shader->setVec4("PaintColor", g_highlightColor);
				glDrawArrays(GL_LINES, 0, 2);
				glLineWidth(2);
				glDisable(GL_DEPTH_TEST);
			}
		}
		if (g_canvasInstance->showInnerPoint)
		{
			g_pointShader->use();
			g_pointShader->setMat4("model", worldModelMatrix);
			g_pointShader->setVec4("PaintColor", g_whiteColor);
			glDrawArrays(GL_POINTS, 0, 2);
		}
	}
}

glm::vec3 Line2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(this->start, 1.0f);
}

glm::vec3 Line2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(this->end, 1.0f);
}

void Line2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}

void Line2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->centroid, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}

void Line2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;
}
void Line2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
	glm::vec3 offsetToOrigin = -center;

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

	worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
}
void Line2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{
	this->start = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->start, 1.0f), linePt1, linePt2);
	this->end = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->end, 1.0f), linePt1, linePt2);
	this->centroid = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->centroid, 1.0f), linePt1, linePt2);
}

void Line2DGPU::SetStartPoint(int index)
{
	if (index == 0)
		return;
	else if (index == 1)
	{
		std::swap(this->start, this->end);
	}
}

void Line2DGPU::Reverse()
{
	std::swap(this->start, this->end);
	UpdatePaintData();
}

glm::vec3 Line2DGPU::Evaluate(float t)
{
	glm::vec3 drv = Derivative(0.0f);
	glm::vec3 evaluated = start + drv * t;
	return worldModelMatrix * glm::vec4(evaluated, 1.0f);
}

glm::vec3 Line2DGPU::Derivative(float t)
{
	return glm::vec3(end.x - start.x, end.y - start.y, 0.0f);
}

float Line2DGPU::Curvature(float t)
{
	return 0.0f;
}

float Line2DGPU::CurvatureRadius(float t)
{
	return FLT_MAX;
}

void Line2DGPU::SetParameter(const glm::vec3& start, const glm::vec3& end)
{
	bbox = AABB(start, end);;
	area = 0;

	this->start = start;
	this->end = end;

	centroid = start + end;
	centroid /= 2;

	boostPath.clear();
	pathLength = glm::distance(start, end);
	bg::append(boostPath, BoostPoint(this->start.x, this->start.y));
	bg::append(boostPath, BoostPoint(this->end.x, this->end.y));
	direction = MathUtils::GetDirection(this->centroid, this->start, this->end);
	UpdatePaintData();
}

std::vector<glm::vec3> Line2DGPU::GetTransformedNodes()
{
	return { this->worldModelMatrix * glm::vec4(this->start,1.0f),this->worldModelMatrix * glm::vec4(this->end,1.0f) };

}

std::vector<glm::vec3> Line2DGPU::SplitToSection(float precision)
{
	float step = precision / pathLength;
	std::vector<glm::vec3> res;
	float t = 0;
	while (t < 1.0)
	{
		res.push_back(Evaluate(t));
		t += step;
	}

	res.push_back(this->worldModelMatrix * glm::vec4(this->end, 1.0f));
	return res;
}

std::string Line2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 start = transformedMatrix * glm::vec4(this->start, 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(this->end, 1.0f);
		
		char buffer[256];

		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			std::sprintf(buffer, "N%03d G00 Z%f\n", Mstatus->ncstep++, Mstatus->Zup);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			std::sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			s += buffer;
			if (createRecord)
			{
				Path2D* path = new Path2D({ Mstatus->toolPos,start }, true);
				path->SetTransformation(glm::mat4(1.0f));
				sketch->AddPath(path);
				GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
				rec.attachedPath = path;
				GCodeController::GetController()->AddRecord(rec);
			}
			std::sprintf(buffer, "N%03d G01 Z%f\n", Mstatus->ncstep++, Mstatus->Zdown);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, end.x, end.y);
		s += buffer;
		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->toolPos = end;
	}

	return s;
}

std::string Line2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string section = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);
		glm::vec3 start = transformedMatrix * glm::vec4(this->start, 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(this->end, 1.0f);

		char buffer[256];
		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}
		std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, end.x, end.y);
		section += buffer;

		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->toolPos = end;
	}
	return section;
}

QString Line2DGPU::Description()
{
	glm::vec3 transformedStart = worldModelMatrix * glm::vec4(start, 1.0f);
	glm::vec3 transformedEnd = worldModelMatrix * glm::vec4(end, 1.0f);
	return QString("线段 序号: %1 起点:(%2,%3) 终点:(%4,%5) 图形总长:%6").
		arg(id).arg(transformedStart.x).arg(transformedStart.y).arg(transformedEnd.x).arg(transformedEnd.y).arg(pathLength);
}

