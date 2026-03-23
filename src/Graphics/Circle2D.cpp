#include "Graphics/Circle2D.h"
#include "Common/Program.h"
#include "Controls/GCodeController.h"
#include "UI/GCodeEditor.h"
#include "Graphics/Sketch.h"

Circle2DGPU::Circle2DGPU()
{

}

Circle2DGPU::Circle2DGPU(Circle2DGPU* other)
{
}

Circle2DGPU::Circle2DGPU(glm::vec3 center, float radius) : center(center), radius(radius)
{
	bbox = AABB(glm::vec3(center.x - radius, center.y - radius, 0.0f), glm::vec3(center.x + radius, center.y + radius, 0.0f));
	GenerateCircleSamplePoints(center, radius, 5, circleSamples);
	area = 2 * (float)PI * radius;

	centroid = glm::vec3(0.0f);
	boostPath.clear();
	for (const glm::vec3& vec : circleSamples)
	{
		bg::append(boostPath, BoostPoint(vec.x, vec.y));
		centroid += vec;
	}
	centroid /= circleSamples.size();
	bbox = AABB(glm::vec3(center.x - radius, center.y - radius, 0.0f), glm::vec3(center.x + radius, center.y + radius, 0.0f));
	pathLength = 2 * PI * radius;

	direction = MathUtils::GetDirection(this->centroid, circleSamples[0], circleSamples[1]);
}
Circle2DGPU::~Circle2DGPU()
{

}

void Circle2DGPU::Copy(Circle2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->center, other->radius);
}

void Circle2DGPU::UpdatePaintData()
{
	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, circleSamples.size() * sizeof(glm::vec3), circleSamples.data(), GL_STATIC_DRAW);
	}

	int i = 0;
	for (int i = 0; i < circleSamples.size(); i++)
	{
		glm::vec3 transformed = worldModelMatrix * glm::vec4(circleSamples[i], 1.0f);
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

glm::vec3 Circle2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(this->circleSamples[0], 1.0f);
}

glm::vec3 Circle2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(this->circleSamples[circleSamples.size() - 1], 1.0f);
}

void Circle2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			GenerateCircleSamplePoints(center, radius, 5, circleSamples);
			glBufferData(GL_ARRAY_BUFFER, circleSamples.size() * sizeof(glm::vec3), circleSamples.data(), GL_STATIC_DRAW);

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

			glLineWidth(g_lineWidth);
			glBindVertexArray(vao);
			glDrawArrays(GL_LINE_STRIP, 0, circleSamples.size());

			if (openHighlight)
			{
				glLineWidth(4.0f);
				shader->setVec4("PaintColor", g_highlightColor);
				glDrawArrays(GL_LINE_STRIP, 0, circleSamples.size());
				glLineWidth(2);
			}
		}
	}
	if (g_canvasInstance->showInnerPoint)
	{
		g_pointShader->use();
		g_pointShader->setMat4("model", worldModelMatrix);
		g_pointShader->setVec4("PaintColor", g_whiteColor);
		glDrawArrays(GL_POINTS, 0, circleSamples.size());
	}
}
void Circle2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}
void Circle2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->center, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}
void Circle2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;
}
void Circle2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
	glm::vec3 offsetToOrigin = -center;

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

	worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
}
void Circle2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{
	std::vector<glm::vec3> transformedNodes = GetTransformedNodes();

	for (int i = 0; i < transformedNodes.size(); i++)
	{
		circleSamples[i] = MathUtils::reflectPoint(transformedNodes[i], linePt1, linePt2);
	}
	this->centroid = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->center, 1.0f), linePt1, linePt2);
	this->center = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->center, 1.0f), linePt1, linePt2);
}

void Circle2DGPU::SetStartPoint(int index)
{
	if (index != 0)
	{
		std::vector<glm::vec3> temp;
		for (int i = index; i < circleSamples.size(); i++)
		{
			temp.push_back(circleSamples[i]);
		}
		for (int i = 0; i < index; i++)
		{
			temp.push_back(circleSamples[i]);
		}
		circleSamples = temp;
	}
}

void Circle2DGPU::Reverse()
{
	std::reverse(circleSamples.begin(), circleSamples.end());
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

glm::vec3 Circle2DGPU::Evaluate(float t)
{
	float angle = t * 360.0f;
	glm::vec3 evaluated = glm::vec3(center.x + radius * (float)cos(angle * deg2Rad), center.y + radius * (float)sin(angle * deg2Rad), 0.0f);
	return this->worldModelMatrix * glm::vec4(evaluated, 1.0f);
}

glm::vec3 Circle2DGPU::Derivative(float t)
{
	float angle = t * 360.0f;
	glm::vec3 drv = glm::vec3(-radius * (float)sin(angle * deg2Rad), radius * (float)cos(angle * deg2Rad), 0.0f);
	return this->worldModelMatrix * glm::vec4(drv, 1.0f);
}

float Circle2DGPU::Curvature(float t)
{
	return 1.0f / radius;
}

float Circle2DGPU::CurvatureRadius(float t)
{
	return radius;
}

QString Circle2DGPU::Description()
{
	glm::vec3 transformedCenter = glm::vec4(center, 1.0f);

	return QString("圆,圆心:(%1,%2),半径:%3 图形总长:%4").arg(transformedCenter.x).arg(transformedCenter.y)
		.arg(radius).arg(pathLength);
}

void Circle2DGPU::SetParameter(const glm::vec3& center, float r)
{
	circleSamples.clear();
	this->center = center;
	this->radius = r;

	GenerateCircleSamplePoints(this->center, this->radius, 5, circleSamples);
	area = 2 * (float)PI * radius;
	bbox = AABB(glm::vec3(center.x - radius, center.y - radius, 1.0f), glm::vec3(center.x + radius, center.y + radius, 1.0f));


	centroid = glm::vec3(0.0f);
	boostPath.clear();
	for (const glm::vec3& vec : circleSamples)
	{
		bg::append(boostPath, BoostPoint(vec.x, vec.y));
		centroid += vec;
	}
	centroid /= circleSamples.size();
	pathLength = 2 * PI * radius;
	UpdatePaintData();
	direction = MathUtils::GetDirection(this->centroid, circleSamples[0], circleSamples[1]);
}

std::vector<glm::vec3> Circle2DGPU::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	for (auto& pt : circleSamples)
	{
		res.push_back(this->worldModelMatrix * glm::vec4(pt, 1.0f));
	}
	return res;
}

std::string Circle2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 center = transformedMatrix * glm::vec4(this->center, 1.0f);

		glm::vec3 start = this->center + glm::vec3(0, radius, 0.0f);
		start = transformedMatrix * glm::vec4(start, 1.0f);
		glm::vec3 end = this->center + glm::vec3(0, radius, 0.0f);
		end = transformedMatrix * glm::vec4(end, 1.0f);


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

		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			std::sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			s += buffer;
			if (createRecord)
			{
				Path2D* path = new Path2D({ Mstatus->toolPos, start }, true);
				path->SetTransformation(glm::mat4(1.0f));
				sketch->AddPath(path);
				GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
				rec.attachedPath = path;
				GCodeController::GetController()->AddRecord(rec);
			}
			std::sprintf(buffer, "N%03d G01 Z%f\n", Mstatus->ncstep++, Mstatus->Zdown);
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			s += buffer;
		}
		std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
		s += buffer;
		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, circleSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->toolPos = end;
	}
	return s;
}

std::string Circle2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string section = "";

	if (createGCode)
	{
		char buffer[256];

		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);
		glm::vec3 center = transformedMatrix * glm::vec4(this->center, 1.0f);

		glm::vec3 start = this->center + glm::vec3(0, radius, 0.0f);
		start = transformedMatrix * glm::vec4(start, 1.0f);
		glm::vec3 end = this->center + glm::vec3(0, radius, 0.0f);
		end = transformedMatrix * glm::vec4(end, 1.0f);

		float I = (center - start).x;
		float J = (center - start).y;

		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			Mstatus->totalPath += glm::distance(start,Mstatus->toolPos);
			Mstatus->idlePath += glm::distance(start, Mstatus->toolPos);
		}

		std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
		section += buffer;
		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, circleSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->totalPath += pathLength;
		Mstatus->toolPos = end;
	}

	return section;
}

std::vector<glm::vec3> Circle2DGPU::SplitToSection(float precision)
{
	float step = precision / (radius * M_PI);
	float t = 0;
	std::vector<glm::vec3> res;

	while (t < 1)
	{
		res.push_back(Evaluate(t));
		t += step;
	}

	res.push_back(worldModelMatrix * glm::vec4(circleSamples[0], 1.0f));
	return res;
}

void Circle2DGPU::GenerateCircleSamplePoints(const glm::vec3& center, float radius, int stepAngle, std::vector<glm::vec3>& samples)
{
	if (samples.size())
		samples.clear();

	samples.reserve(360 / stepAngle + 1);
	for (int angle = 0; angle <= 360; angle += stepAngle)
	{
		float x = (radius * cos(angle * deg2Rad));
		float y = (radius * sin(angle * deg2Rad));

		samples.push_back(glm::vec3(center.x + x, center.y + y, 0.0));
	}

	indexRange = { 0,samples.size() - 1 };
}