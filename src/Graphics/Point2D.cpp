#include "Graphics/Point2D.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define POINT_RADIUS 0.05f

Point2DGPU::Point2DGPU()
{
}
Point2DGPU::Point2DGPU(Point2DGPU* other) : EntityVGPU(other)
{
	this->SetParameter(other->point);
}
Point2DGPU::Point2DGPU(const glm::vec3& point)
{
	this->point = point;
	this->bbox = new AABB(point- glm::vec3(POINT_RADIUS, POINT_RADIUS, 0.0f), point+ glm::vec3(POINT_RADIUS, POINT_RADIUS, 0.0f));
	centroid = point;

	boostPath.clear();
	//给一个极小的范围否则rTree检索不出
	bg::append(boostPath, BoostPoint(this->point.x - POINT_RADIUS, this->point.y - POINT_RADIUS));
	bg::append(boostPath, BoostPoint(this->point.x + POINT_RADIUS, this->point.y + POINT_RADIUS));
	indexRange = { 0,0 };
}
Point2DGPU::~Point2DGPU()
{

}
void Point2DGPU::Copy(Point2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->point);
}
void Point2DGPU::UpdatePaintData()
{
	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &point, GL_STATIC_DRAW);
	}

	//this->point = worldModelMatrix * glm::vec4(this->point, 1.0f);
	glm::vec3 temp = worldModelMatrix * glm::vec4(this->point, 1.0f);
	bg::set<0>(boostPath[0], temp.x - POINT_RADIUS);
	bg::set<1>(boostPath[0], temp.y - POINT_RADIUS);
	bg::set<0>(boostPath[1], temp.x + POINT_RADIUS);
	bg::set<1>(boostPath[1], temp.y + POINT_RADIUS);

	BoostBox boostBox;
	bg::envelope(boostPath, boostBox);
	BoostPoint min = boostBox.min_corner();
	BoostPoint max = boostBox.max_corner();
	bbox = AABB(glm::vec3(bg::get<0>(min), bg::get<1>(min), 0.0f), glm::vec3(bg::get<0>(max), bg::get<1>(max), 0.0f));
	this->modelMatrixStash = this->worldModelMatrix;
}
void Point2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(point), GL_STATIC_DRAW);
		}
		if (isHover)
		{
			color = g_yellowColor;
		}
		shader->use();
		shader->setMat4("model", worldModelMatrix);
		shader->setVec4("PaintColor", color);
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, 1);
	}
}
glm::vec3 Point2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(point,1.0f);
}
glm::vec3 Point2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(point,1.0f);
}
void Point2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}

void Point2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->point, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}

void Point2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;
}
void Point2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
}
void Point2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{

}

void Point2DGPU::SetStartPoint(int index)
{
}

void Point2DGPU::Reverse()
{
}

glm::vec3 Point2DGPU::Evaluate(float t)
{
	return this->worldModelMatrix * glm::vec4(point, 1.0f);
}

glm::vec3 Point2DGPU::Derivative(float t)
{
	return glm::vec3(0.0f);
}

float Point2DGPU::Curvature(float t)
{
	std::cerr << "点没有曲率属性" << std::endl;
	return 0.0f;
}

float Point2DGPU::CurvatureRadius(float t)
{
	std::cerr << "点没有曲率半径属性" << std::endl;
	return 0.0f;
}

void Point2DGPU::SetParameter(const glm::vec3& point)
{
	this->point = point;
	bbox = AABB(point, point);
	boostPath.clear();
	bg::append(boostPath, BoostPoint(this->point.x - POINT_RADIUS, this->point.y - POINT_RADIUS));
	bg::append(boostPath, BoostPoint(this->point.x + POINT_RADIUS, this->point.y + POINT_RADIUS));
	UpdatePaintData();
}

std::string Point2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	return "";
}

std::string Point2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	return std::string();
}

QString Point2DGPU::Description()
{
	return QString("点 坐标:(%1,%2)").arg(point.x).arg(point.y);
}

std::vector<glm::vec3> Point2DGPU::SplitToSection(float precision)
{
	return std::vector<glm::vec3>({ point });
}