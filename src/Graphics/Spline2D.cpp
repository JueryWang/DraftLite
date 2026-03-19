#include "Graphics/Spline2D.h"
#include "Common/ProgressInfo.h"
#include "Controls/GCodeController.h"
#include "UI/GCodeEditor.h"
#include "Graphics/Sketch.h"

Spline2DGPU::Spline2DGPU()
{
}

Spline2DGPU::Spline2DGPU(Spline2DGPU* other) : EntityVGPU(other)
{
	this->SetParameter(other->controlPoints, other->knots, other->isPassthrough);
}

Spline2DGPU::Spline2DGPU(const std::vector<glm::vec3>& controlPoints, const std::vector<float> knots, bool isPassTrough) : controlPoints(controlPoints), knots(knots), isPassthrough(isPassTrough)
{

	if (!isPassthrough)
	{
		GenerateSplineSamplePoints(controlPoints, splineSamples);
	}
	else
	{
		this->splineSamples = MathUtils::CatmullRomSmooth(controlPoints,1);
		for (int i = 0; i < splineSamples.size(); i++)
		{
			PathNode p;
			p.Node = splineSamples[i];
			pathNodes.push_back(p);
		}

		indexRange = { 0,splineSamples.size() - 1 };
	}
	this->controlPoints = controlPoints;
	this->knots = knots;
	bbox = AABB(splineSamples[0], splineSamples[1]);

	boostPath.clear();
	for (glm::vec3& p : splineSamples)
	{
		bbox.Union(p);
		bg::append(boostPath, BoostPoint(p.x, p.y));
	}

	bool Closed = (splineSamples[0] == splineSamples[splineSamples.size() - 1]);

	this->area = Closed ? MathUtils::ComputeArea(this->GetTransformedNodes()) : 0;
	bg::correct(boostPath);
	UpdatePaintData();

	centroid = glm::vec3(0.0f);
	for (const glm::vec3& p : controlPoints)
	{
		centroid += p;
	}
	centroid /= controlPoints.size();

	pathLength = 0;
	for (int i = 0; i < splineSamples.size() - 1;i++)
	{
		pathLength += glm::distance(splineSamples[i],splineSamples[i+1]);
	}

	int i = 0;
	direction = MathUtils::GetDirection(this->centroid, controlPoints[0], controlPoints[1]);
	while (direction == GeomDirection::AtLine && i < controlPoints.size() - 2)
	{
		i++;
		direction = MathUtils::GetDirection(this->centroid, controlPoints[i], controlPoints[i + 1]);
	}
}
Spline2DGPU::~Spline2DGPU()
{

}
void Spline2DGPU::Copy(Spline2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->controlPoints, other->knots, other->isPassthrough);
}
void Spline2DGPU::UpdatePaintData()
{
	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, splineSamples.size() * sizeof(glm::vec3), splineSamples.data(), GL_STATIC_DRAW);
	}

	int i = 0;
	for (int i = 0; i < splineSamples.size(); i++)
	{
		glm::vec3 transformed = worldModelMatrix * glm::vec4(splineSamples[i], 1.0f);
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
glm::vec3 Spline2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(this->splineSamples[0], 1.0f);
}
glm::vec3 Spline2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(this->splineSamples.back(), 1.0f);
}
void Spline2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glBufferData(GL_ARRAY_BUFFER, splineSamples.size() * sizeof(glm::vec3), splineSamples.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);

			glGenVertexArrays(1, &vao_cntl);
			glBindVertexArray(vao_cntl);

			glGenBuffers(1, &vbo_cntl);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_cntl);
			glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec3), controlPoints.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), 0);
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
			glDrawArrays(GL_LINE_STRIP, 0, splineSamples.size());

			if (openHighlight)
			{
				glLineWidth(4.0f);
				shader->setVec4("PaintColor", g_highlightColor);
				glDrawArrays(GL_LINE_STRIP, highlightSection.first, (highlightSection.second - highlightSection.first + 1));
				glLineWidth(2.0f);
			}
		}
		if (g_canvasInstance->showInnerPoint)
		{
			g_pointShader->use();
			g_pointShader->setMat4("model", worldModelMatrix);
			g_pointShader->setVec4("PaintColor", g_whiteColor);
			glDrawArrays(GL_POINTS, 0, splineSamples.size());
		}
	}
}
void Spline2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}
void Spline2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->centroid, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}
void Spline2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;

}
void Spline2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
	glm::vec3 offsetToOrigin = -center;

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

	worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
}
void Spline2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{
	std::vector<glm::vec3>  transformedNodes = GetTransformedNodes();

	for (auto& pt : controlPoints)
	{
		pt = MathUtils::reflectPoint(glm::vec4(pt, 1.0f), linePt1, linePt2);
	}
	for (int i = 0; i < transformedNodes.size(); i++)
	{
		splineSamples[i] = MathUtils::reflectPoint(transformedNodes[i], linePt1, linePt2);
	}
	this->centroid = MathUtils::reflectPoint(glm::vec4(this->centroid, 1.0f), linePt1, linePt2);
}

void Spline2DGPU::SetStartPoint(int index)
{
	std::cerr << "无法设置样条的起始点" << std::endl;
}

void Spline2DGPU::Reverse()
{
	std::reverse(controlPoints.begin(), controlPoints.end());
	std::reverse(splineSamples.begin(), splineSamples.end());
	std::reverse(pathNodes.begin(), pathNodes.end());
	std::reverse(knots.begin(), knots.end());
	for (float& t : samplesT)
	{
		t = 1.0f - t;
	}

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

glm::vec3 Spline2DGPU::Evaluate(float t)
{
	glm::vec3 sample = MathUtils::CalculateBSpline(controlPoints, knots, 3, t);
	return this->worldModelMatrix * glm::vec4(sample, 1.0f);
}

glm::vec3 Spline2DGPU::Derivative(float t)
{
	float dt = 1e-4f;
	glm::vec3 p1 = Evaluate(t);
	float nextT = (t - dt) > 0.0f ? (t - dt) : 1.0f;
	glm::vec3 p2 = Evaluate(nextT > 0.0f ? nextT : 0.0f);
	return (p2 - p1) / dt;
}

float Spline2DGPU::Curvature(float t)
{
	return 1.0f / CurvatureRadius(t);
}

float Spline2DGPU::CurvatureRadius(float t)
{
	int index = 0;
	float cumSum = 0;
	std::vector<float> samplesT;

	while ((index + 1) < splineSamples.size())
	{
		samplesT.push_back(cumSum / pathLength);
		cumSum += glm::distance(splineSamples[index], splineSamples[(index + 1) % splineSamples.size()]);
		index++;
	}

	auto it = std::upper_bound(samplesT.begin(), samplesT.end(), t);
	int itIndex = std::distance(samplesT.begin(), it);

	while (itIndex < 1)
		itIndex++;
	while (itIndex > (samplesT.size() - 2))
		itIndex--;

	glm::vec3 center;
	float radius;
	float startAngle;
	float endAngle;

	std::tie(center, startAngle, endAngle, radius) = MathUtils::CalculateCircleByThreePoints(splineSamples[(itIndex - 1)], splineSamples[itIndex], splineSamples[(itIndex + 1)]);

	glm::vec3 p1 = Evaluate(t - 0.01f);
	glm::vec3 p2 = Evaluate(t);
	glm::vec3 p3 = Evaluate(t + 0.01f);

	//glm::vec3 center2;
	//float radius2;
	//float startAngle2;
	//float endAngle2;

	//std::tie(center2, startAngle2, endAngle2, radius2) = MathUtils::CalculateCircleByThreePoints(p1, p2, p3);
	//std::cout << "radius:" << radius << ", radius2:" << radius2 << std::endl;
	return radius;
}

std::vector<glm::vec3> Spline2DGPU::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	res.reserve(splineSamples.size());
	for (auto& pt : splineSamples)
	{
		res.push_back(this->worldModelMatrix * glm::vec4(pt, 1.0f));
	}
	return res;
}

float Spline2DGPU::findT(const float lastT, const int precision)
{
	float left = lastT;
	float right = 1.0f;
	float target = static_cast<float>(precision);
	float eps = target * 0.001f;
	int maxIter = 32;
	float t = left;

	for (int i = 0; i < maxIter; ++i) {
		t = (left + right) * 0.5f;
		float cum = cumulativeLength(lastT, t);
		float diff = cum - target;
		if (fabs(diff) < eps)
			return t;
		if (diff > 0)
			right = t;
		else
			left = t;
	}
	return t;
}

std::vector<glm::vec3> Spline2DGPU::SplitToSection(float precision)
{
	std::vector<glm::vec3> res;
	res.push_back(this->worldModelMatrix * glm::vec4(splineSamples[0], 1.0f));

	std::vector<float> newSamplesT;

	float lastT = 0.0f;
	float stepT = precision / pathLength;

	do
	{
		float t = findT(lastT, precision);
		newSamplesT.push_back(t);
		res.push_back(this->worldModelMatrix * glm::vec4(Evaluate(t), 1.0f));
		lastT = t;
	} while (lastT < 1.0f);

	int iStart = 0;

	//增加采样密度,使得分割出的点在样条上
	for (auto& t : newSamplesT)
	{
		for (int i = iStart; i < samplesT.size() - 1; i++)
		{
			if (t > samplesT[i] && t < samplesT[i + 1])
			{
				iStart = i;
				samplesT.insert(samplesT.begin() + i + 1, t);
				splineSamples.insert(splineSamples.begin() + i + 1, Evaluate(t));
				break;
			}
		}
	}

	return res;
}

QString Spline2DGPU::Description()
{
	QString description;
	if (editPointIndex < 0)
	{
		if (splineSamples[0] == splineSamples[splineSamples.size() - 1])
		{
			description = QString("多段Bezier曲线(%1) %2段 图形总长:%3").arg("闭合").arg(controlPoints.size()).arg(pathLength);
		}
		else
		{
			description = QString("多段Bezier曲线(%1) %2段 图形总长:%3").arg("非闭合").arg(controlPoints.size()).arg(pathLength);
		}
	}
	else
	{
		float cumsum = 0;
		int index = 0;
		while (index < editPointIndex)
		{
			cumsum += glm::distance(splineSamples[index], splineSamples[index + 1]);
			index++;
		}
		glm::vec3 tangentPre = pathNodes[editPointIndex].Tangent;
		glm::vec3 tangentNext = pathNodes[(editPointIndex + 1) % pathNodes.size()].Tangent;
		float tan = fabs(MathUtils::GetTangentValue(tangentPre, tangentNext));

		glm::vec3 editPointPos = worldModelMatrix * glm::vec4(splineSamples[editPointIndex], 1.0f);
		QString PointDesc = QString(",编辑点(%1):(%2,%3)").arg(editPointIndex).arg(editPointPos.x).arg(editPointPos.y);
		description = QString("index = %1, tangentValue = %2 %3").arg(editPointIndex).arg(tan).arg(PointDesc);
	}

	return description;
}

void Spline2DGPU::SetParameter(const std::vector<glm::vec3>& controlpoints, const std::vector<float>& knots, bool isPassthrough)
{
	this->controlPoints = controlpoints;
	this->knots = knots;
	this->isPassthrough = isPassthrough;

	if (!isPassthrough)
	{
		GenerateSplineSamplePoints(controlPoints, splineSamples);
	}
	else
	{
		splineSamples = MathUtils::CatmullRomSmooth(controlPoints, 100);
		for (int i = 0; i < splineSamples.size(); i++)
		{
			PathNode p;
			p.Node = splineSamples[i];
			pathNodes.push_back(p);
		}

		indexRange = { 0,splineSamples.size() - 1 };
	}

	bbox = AABB(controlPoints[0], controlPoints[1]);
	for (int i = 1; i < controlPoints.size(); i++)
	{
		bbox.Union(controlPoints[i]);
	}

	centroid = glm::vec3(0.0f);
	this->boostPath.clear();
	for (glm::vec3& p : splineSamples)
	{
		centroid += p;
		bg::append(this->boostPath, BoostPoint(p.x, p.y));
	}
	centroid /= controlPoints.size();

	bool Closed = (splineSamples[0] == splineSamples[splineSamples.size() - 1]);
	this->area = Closed ? MathUtils::ComputeArea(this->GetTransformedNodes()) : 0;

	pathLength = 0;
	int size = splineSamples.size();
	for (int i = 0; i < size - 1; i++)
	{
		pathLength += glm::distance(splineSamples[i], splineSamples[i + 1]);
	}
	pathLength += glm::distance(splineSamples[size - 1], splineSamples[0]);
	UpdatePaintData();

	int i = 0;
	direction = MathUtils::GetDirection(this->centroid, controlPoints[0], controlPoints[1]);
	while (direction == GeomDirection::AtLine && i < controlPoints.size() - 2)
	{
		i++;
		direction = MathUtils::GetDirection(this->centroid, controlPoints[i], controlPoints[i + 1]);
	}
}

float Spline2DGPU::cumulativeLength(double t0, double t1, double eps)
{
	/*
	* Simpson积分法,精度达标情况下计算效率太低
	if (t <= 0) return 0.0f;
	auto f = [this](double x) {
		return glm::length(Derivative(x));
	};
	return MathUtils::SimpsonIntegrate(f, t0, t, eps, 15);
	*/
	float res = 0.0f;
	float left, right;
	int iLeft, iRight;
	for (int i = 0; i < samplesT.size(); i++)
	{
		float t = samplesT[i];
		if (t <= t0)
		{
			left = t;
			iLeft = i;
		}
		if (t >= t1)
		{
			right = t;
			iRight = i;
			break;
		}
	}
	if ((iRight - iLeft) > 2)
	{
		float u = (samplesT[iLeft + 1] - t0) / (samplesT[iLeft + 1] - samplesT[iLeft]);
		res += u * (glm::length(splineSamples[iLeft + 1] - splineSamples[iLeft]));
		for (int i = iLeft + 1; i < (iRight - 1); i++)
		{
			res += glm::length(splineSamples[i + 1] - splineSamples[i]);
		}
		u = (t1 - samplesT[iRight - 1]) / (samplesT[iRight] - samplesT[iRight - 1]);
		res += u * (glm::length(splineSamples[iRight] - splineSamples[iRight - 1]));
	}
	else
	{
		float u = (t1 - t0) / (samplesT[iRight] - samplesT[iLeft]);
		res = u * (glm::length(splineSamples[iRight] - splineSamples[iLeft]));
	}

	return res;
}

std::string Spline2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);


		glm::vec3 start = transformedMatrix * glm::vec4(splineSamples[0], 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(splineSamples[splineSamples.size() - 1], 1.0f);

		char buffer[256];
		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			std::sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}

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
				//Path2D* path = new Path2D({ Mstatus->toolPos,start }, true);
				//path->SetTransformation(glm::mat4(1.0f));
				//sketch->AddPath(path);
				//GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
				//rec.attachedPath = path;
				//GCodeController::GetController()->AddRecord(rec);
			}
			std::sprintf(buffer, "N%03d G01 Z%f\n", Mstatus->ncstep++, Mstatus->Zdown);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
		}

		glm::vec3 transformed;
		int step = 10;

		transformed = transformedMatrix * glm::vec4(splineSamples[0], 1.0f);
		std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
		s += buffer;

		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}

		const int compensation = 2;
		const float cornerThreadshold = 1e2; //30度夹角余弦值
		glm::vec3 nodePre = transformedMatrix * glm::vec4(pathNodes[0].Node, 1.0f);

		for (int i = 1; i < pathNodes.size(); i += step)
		{
			transformed = transformedMatrix * glm::vec4(pathNodes[i].Node, 1.0f);
			if (glm::distance(nodePre, transformed) > 0.001)
			{
				std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
				s += buffer;
				if (createRecord)
				{
					GCodeRecord rec(std::string(buffer), this, i, transformedMatrix, Mstatus->ncstep);
					GCodeController::GetController()->AddRecord(rec);
				}

				//拐角前后采样compensation个点
				for (int j = 0; j < step && (i + j + compensation) < pathNodes.size(); j++)
				{
					glm::vec3 tangentPre = pathNodes[i + j].Tangent;
					glm::vec3 tangentNext = pathNodes[(i + 1 + j) % pathNodes.size()].Tangent;

					float tan = fabs(MathUtils::GetTangentValue(tangentPre, tangentNext));
					if (tan > cornerThreadshold)
					{
						for (int k = -compensation; k <= compensation; k++)
						{
							int index = std::clamp(i + j + k, i, i + step);
							transformed = transformedMatrix * glm::vec4(pathNodes[index].Node, 1.0f);
							std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
							s += buffer;
							if (createRecord)
							{
								GCodeRecord rec(std::string(buffer), this, i, transformedMatrix, Mstatus->ncstep);
								GCodeController::GetController()->AddRecord(rec);
							}
						}
						j += compensation;
					}
				}
			}
			nodePre = transformed;
		}
		transformed = transformedMatrix * glm::vec4(splineSamples[splineSamples.size() - 1], 1.0f);
		std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
		s += buffer;

		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, splineSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->toolPos = transformed;
	}
	return s;
}

std::string Spline2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string section = "";
	if (createGCode)
	{
		char buffer[256];
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 start = transformedMatrix * glm::vec4(splineSamples[0], 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(splineSamples[splineSamples.size() - 1], 1.0f);

		if (glm::distance(start, Mstatus->toolPos) > CONNECT_EPSILON)
		{
			std::sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), nullptr, -1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			Mstatus->totalPath += glm::distance(start, Mstatus->toolPos);
			Mstatus->idlePath += glm::distance(start, Mstatus->toolPos);
		}
		glm::vec3 transformed;
		int step = 10;

		transformed = transformedMatrix * glm::vec4(splineSamples[0], 1.0f);
		std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, start.x, start.y);
		section += buffer;

		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}

		const int compensation = 2;
		const float cornerThreadshold = 1e2; //30度夹角余弦值

		for (int i = 1; i < pathNodes.size(); i += step)
		{
			transformed = transformedMatrix * glm::vec4(pathNodes[i].Node, 1.0f);
			std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
			section += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, i, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}

			//拐角前后采样compensation个点
			for (int j = 0; j < step && (i + j + compensation) < pathNodes.size(); j++)
			{
				glm::vec3 tangentPre = pathNodes[i + j].Tangent;
				glm::vec3 tangentNext = pathNodes[(i + 1 + j) % pathNodes.size()].Tangent;
				float tan = fabs(MathUtils::GetTangentValue(tangentPre, tangentNext));
				if (tan > cornerThreadshold)
				{
					for (int k = -compensation; k <= compensation; k++)
					{
						int index = std::clamp(i + j + k, i, i + step);
						transformed = transformedMatrix * glm::vec4(pathNodes[index].Node, 1.0f);
						std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
						section += buffer;
						if (createRecord)
						{
							GCodeRecord rec(std::string(buffer), this, i, transformedMatrix, Mstatus->ncstep);
							GCodeController::GetController()->AddRecord(rec);
						}
					}
					j += compensation;
				}
			}
		}
		transformed = transformedMatrix * glm::vec4(splineSamples[splineSamples.size() - 1], 1.0f);
		std::sprintf(buffer, "N%03d G05 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
		section += buffer;

		if (createRecord)
		{
			GCodeRecord rec(std::string(buffer), this, splineSamples.size() - 1, transformedMatrix, Mstatus->ncstep);
			GCodeController::GetController()->AddRecord(rec);
		}
		Mstatus->totalPath += pathLength;
		Mstatus->toolPos = end;
	}

	return section;
}

void Spline2DGPU::GenerateSplineSamplePoints(const std::vector<glm::vec3>& controlPoints, std::vector<glm::vec3>& samples)
{
	if (samples.size() != 0)
		samples.clear();
	samples.reserve(controlPoints.size() * 10);

	glm::vec3 firstPoint = MathUtils::CalculateBSpline(controlPoints, knots, 3, 0);
	bbox = AABB(firstPoint, firstPoint);

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
		samplesT.push_back(t);
	}
	glm::vec3 lastPoint = MathUtils::CalculateBSpline(controlPoints, knots, 3, 1.0f);
	samples.push_back(lastPoint);
	samplesT.push_back(1.0f);
	bbox.Union(lastPoint);

	lastPoint = splineSamples[0];
	for (int i = 0; i < splineSamples.size(); i++)
	{
		PathNode p;
		p.Node = splineSamples[i];
		p.Normal = this->Normal(samplesT[i]);
		p.Tangent = this->Tangent(samplesT[i]);
		if (i == 0 || lastPoint != splineSamples[i])
		{
			pathNodes.push_back(p);
		}
		lastPoint = splineSamples[i];
	}

	indexRange = { 0,samples.size() - 1 };
}