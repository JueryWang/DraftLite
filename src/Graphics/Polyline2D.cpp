#include "Graphics/Polyline2D.h"
#include "Graphics/Circle2D.h"
#include "Graphics/Point2D.h"
#include "Common/ProgressInfo.h"
#include "Controls/GCodeController.h"
#include "Graphics/Sketch.h"
#include "Graphics/Arc2D.h"
#include "Graphics/OCS.h"
#include "UI/GCodeEditor.h"
#include "Algorithm/ClusterAlgo.h"
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

std::vector<glm::vec3> intermidatePolygon;

Polyline2DGPU::Polyline2DGPU()
{
}

Polyline2DGPU::Polyline2DGPU(Polyline2DGPU* other) : EntityVGPU(other)
{
	this->SetParameter(other->nodes, other->isClosed, other->bulges);
}

Polyline2DGPU::Polyline2DGPU(const std::vector<glm::vec3>& points, bool isClosed, const std::vector<float> _bulge) : nodes(points), isClosed(isClosed), bulges(_bulge)
{
	if (points.size() <= 1)
		return;

	if (nodes[0] == *(nodes.end() - 1))
		isClosed = true;

	//去除末尾的0防止扰乱Reverse的顺序
	if (bulges.size() == nodes.size())
	{
		bulges.erase(bulges.end() - 1);
	}
	GenerateBulgeSamples(nodes, bulges, polylineBulgedSamples);
	if (polylineBulgedSamples.size() <= 2)
	{
		polylineBulgedSamples = nodes;
	}

	bbox = AABB(polylineBulgedSamples[0], polylineBulgedSamples[1]);
	for (int i = 2; i < nodes.size(); i++)
	{
		bbox.Union(nodes[i]);
	}

	centroid = glm::vec3(0.0f);
	boostPath.clear();
	for (glm::vec3& p : nodes)
	{
		bg::append(boostPath, BoostPoint(p.x, p.y));
		centroid += p;
	}
	if (isClosed)
	{
		bg::append(boostPath, BoostPoint(nodes[0].x, nodes[0].y));
		centroid += nodes[0];
	}
	centroid /= nodes.size();
	area = isClosed ? 0 : MathUtils::ComputeArea(this->GetTransformedNodes());//polyon换成linestring需要自己计算area

	int i = 0;
	direction = MathUtils::GetDirection(this->centroid, nodes[0], nodes[1]);
	while (direction == GeomDirection::AtLine && i < nodes.size() - 2)
	{
		i++;
		direction = MathUtils::GetDirection(this->centroid, nodes[i], nodes[i + 1]);
	}
	indexRange = { 0,nodes.size() - 1 };
}
Polyline2DGPU::~Polyline2DGPU()
{
}
void Polyline2DGPU::Copy(Polyline2DGPU* other)
{
	this->EntityVGPU::Copy(other);
	this->SetParameter(other->nodes, other->isClosed, other->bulges);
}
void Polyline2DGPU::ExtendStart(float distance)
{
	std::vector<glm::vec3> polynodes = GetTransformedNodes();
	glm::vec3 start1 = polynodes[0];
	glm::vec3 start2 = polynodes[1];

	glm::vec3 vecTouchStarts = glm::normalize(start1 - start2);
	glm::vec3 startExtend = start1 + distance * vecTouchStarts;
	glm::vec3 StartAdded = glm::inverse(this->worldModelMatrix) * glm::vec4(startExtend,1.0f);
	nodes.insert(nodes.begin(),StartAdded);
	bulges.insert(bulges.begin(),0.0f);
	this->SetParameter(nodes, isClosed,bulges);
}
void Polyline2DGPU::ExtendEnd(float distance)
{
	std::vector<glm::vec3> polynodes = GetTransformedNodes();
	glm::vec3 end1 = polynodes[polynodes.size()-1];
	glm::vec3 end2 = polynodes[polynodes.size()-2];

	glm::vec3 vecTouchEnd = glm::normalize(end1 - end2);
	glm::vec3 EndExtend = end1 + distance * vecTouchEnd;
	glm::vec3 EndAdded = glm::inverse(this->worldModelMatrix) * glm::vec4(EndExtend, 1.0f);
	nodes.push_back(EndAdded);
	bulges.push_back(0.0f);
	this->SetParameter(nodes, isClosed,bulges);
}
void Polyline2DGPU::SelfFitArc(int modifyHint)
{
	//根据曲率半径是否平稳变化来反应是否进入圆弧段

	int windowsize = 10;
	int halfWindowsize = windowsize / 2;
	std::vector<glm::vec3> transformedNodes = GetTransformedNodes();
	std::vector<double> curvatureRadius(transformedNodes.size(),0);
	int size = transformedNodes.size();
	QLineSeries* series = new QLineSeries();
	double maxValue = 0.0f;
	int range = size - 1;
	for (int i = 0; i < transformedNodes.size(); i++)
	{
		double radius = MathUtils::GetCurvatureRadius(transformedNodes[i], transformedNodes[(i+1)% range], transformedNodes[(i+2)% range]);
		curvatureRadius[i] = radius;
	}

	//计算一次差分
	for(int i = 1; i < curvatureRadius.size(); i++)
	{
		curvatureRadius[i] = fmax((curvatureRadius[i] - curvatureRadius[i - 1]), 0);
		series->append(i, curvatureRadius[i]);
	}

	int spilkeCount = 0;
	std::vector<std::pair<int, double>> features;
	std::vector<double> confidences(curvatureRadius.size(), 0);

	for(int i = 0 ; i< curvatureRadius.size(); i++)
	{
		double maxElement = 0.0;
		double localmean = 0.0f;
		for (int j = -halfWindowsize; j < halfWindowsize; j++)
		{
			int index = (i + j + size) % range;
			double value = curvatureRadius[index];
			localmean += value;
			if(maxElement < value)
			{
				maxElement = value;
			}
		}
		localmean = localmean - maxElement;
		localmean /= (windowsize-1);
		double confidence = fmax((curvatureRadius[i] - localmean)/(curvatureRadius[i]+localmean),0);
		features.emplace_back(i,confidence);
		series->append(i, confidence);
		confidences[i] = confidence;
	}
	std::nth_element(confidences.begin(), confidences.begin() + (2*modifyHint) - 1, confidences.end(), std::greater<double>());

	series->setName("曲率半径变化图");

	QChart* chart = new QChart();
	QValueAxis* axisX = new QValueAxis();
	axisX->setRange(0, curvatureRadius.size());

	QValueAxis* axisY = new QValueAxis();
	axisY->setRange(0, maxValue * 1.1);
	chart->setAxisX(axisX, series);
	chart->setAxisY(axisY, series);
	chart->addSeries(series);
	chart->legend()->setVisible(true);
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing); // 抗锯齿，让图表更清晰
	chartView->show();
	chartView->raise();
	chartView->setInteractive(true);
	chartView->setRubberBand(QChartView::RectangleRubberBand);

	double threshold = confidences[(2 * modifyHint -1)];
	std::vector<glm::vec3> newNodes;
	//弧段边界
	std::vector<std::pair<int, int>> ArcBondary;

	std::pair<int, int> currentArc = { -1,-1 };
	for (std::pair<int, double> feature : features)
	{
		if (currentArc.first == -1)
		{
			currentArc.first = feature.first + 2;
		}
		else if (currentArc.first != -1 && feature.second > threshold)
		{
			currentArc.second = feature.first-2;
			ArcBondary.push_back(currentArc);
		}
		if (currentArc.first != -1 && currentArc.second != -1)
		{
			currentArc = { -1,-1 };
		}
	}
	
	for (auto& boundary : ArcBondary)
	{
		if ((boundary.second - boundary.first) > 3)
		{
			Polyline2DGPU* arcSection = new Polyline2DGPU();
			std::vector<glm::vec3> arcNodes;
			for(int i = boundary.first; i <= boundary.second; i++)
			{
				arcNodes.push_back(transformedNodes[i % range]);
			}
			//arcSection->SetParameter(arcNodes,false);
			//arcSection->attribColor = GetRandomColor();
			//arcSection->ResetColor();
			//g_canvasInstance->GetSketchShared()->AddEntity(arcSection);

		}
	}

	std::vector<glm::vec3> fittedNodes;
	std::vector<float> fiitedBulges;
	for (int i = 0; i < transformedNodes.size(); i++)
	{
		bool findInArc = false;
		std::vector<glm::vec3> fitSamples;
		for (auto& bound : ArcBondary)
		{
			if (i == bound.first && i < bound.second)
			{
				findInArc = true;
				for (int j = bound.first; j < bound.second; j++)
				{
					fitSamples.push_back(transformedNodes[j]);
				}
				i = bound.second;
				break;
			}
		}

		if (findInArc)
		{
			//最小二乘法
			glm::vec3 circleCenter;
			double circleRadius;
			std::tie(circleCenter, circleRadius) = MathUtils::FitCircleByPoints(fitSamples);
			double startAngle = glm::degrees(std::atan2(fitSamples[0].y - circleCenter.y,fitSamples[0].x - circleCenter.x));
			if (startAngle < 0)
				startAngle = startAngle + 360;
			double endAngle = glm::degrees(std::atan2(fitSamples.back().y - circleCenter.y, fitSamples.back().x - circleCenter.x));
			if (endAngle < 0)
				endAngle = endAngle + 360;
			if (endAngle < startAngle)
				std::swap(startAngle,endAngle);
			Arc2DGPU* arc = new Arc2DGPU();
			arc->SetParameter(circleCenter, startAngle, endAngle, circleRadius);
			g_canvasInstance->GetSketchShared()->AddEntity(arc);
		}
		else
		{
			fittedNodes.push_back(transformedNodes[i]);
			fiitedBulges.push_back(0.0f);
		}
	}


}
void Polyline2DGPU::UpdatePaintData()
{
	if (vao > 0 && vbo > 0)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, polylineBulgedSamples.size() * sizeof(glm::vec3), polylineBulgedSamples.data(), GL_STATIC_DRAW);
	}

	int i = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		glm::vec3 transformed = worldModelMatrix * glm::vec4(nodes[i], 1.0f);
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
glm::vec3 Polyline2DGPU::GetStart()
{
	return this->worldModelMatrix * glm::vec4(nodes[0], 1.0f);
}
glm::vec3 Polyline2DGPU::GetEnd()
{
	return this->worldModelMatrix * glm::vec4(nodes[nodes.size() - 1], 1.0f);
}
void Polyline2DGPU::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isVisible)
	{
		if (vao == 0 || vbo == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);
			glBufferData(GL_ARRAY_BUFFER, polylineBulgedSamples.size() * sizeof(glm::vec3), polylineBulgedSamples.data(), GL_STATIC_DRAW);
		}

		shader->use();
		shader->setMat4("model", worldModelMatrix);
		shader->setVec4("PaintColor", color);
		if (mode != RenderMode::Point)
		{
			glLineWidth(g_lineWidth);
			glBindVertexArray(vao);
			glDrawArrays(isClosed ? GL_LINE_LOOP : GL_LINE_STRIP, 0, polylineBulgedSamples.size());

			if (openHighlight)
			{
				glLineWidth(4.0f);
				shader->setVec4("PaintColor", g_highlightColor);
				if (highlightSection.second < (polylineBulgedSamples.size()))
				{
					glDrawArrays(GL_LINE_STRIP, std::max(highlightSection.first, 0), (highlightSection.second - highlightSection.first + 1));
				}
				else
				{
					glDrawArrays(GL_LINE_LOOP, 0, polylineBulgedSamples.size());
				}

				glLineWidth(2);
			}
			else
			{
				highlightSection = { -1,-1 };
				glUniform2i(glGetUniformLocation(shader->ID, "highlightSec"), -1, -1);
			}
		}
		if (g_canvasInstance->showInnerPoint)
		{
			g_pointShader->use();
			g_pointShader->setMat4("model", worldModelMatrix);
			g_pointShader->setVec4("PaintColor", g_whiteColor);
			glDrawArrays(GL_POINTS, 0, polylineBulgedSamples.size());
		}
	}
}
void Polyline2DGPU::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
}
void Polyline2DGPU::MoveTo(const glm::vec3& pos)
{
	glm::vec3 worldCentroid = worldModelMatrix * glm::vec4(this->centroid, 1.0f);
	glm::vec3 offset = pos - worldCentroid;
	Move(offset);
}
void Polyline2DGPU::Rotate(const glm::vec3& center, float angle)
{
	float rotateDeg = glm::radians(angle);
	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotateDeg, glm::vec3(0, 0, 1));
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

	glm::mat4 transform = translateBack * rotation * translateToOrigin;

	worldModelMatrix = transform * worldModelMatrix;
}
void Polyline2DGPU::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
	glm::vec3 offsetToOrigin = -center;

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), offsetToOrigin);
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scalar);
	glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offsetToOrigin);

	worldModelMatrix = translateBack * scalingMatrix * translateToOrigin * modelMatrixStash;
}
void Polyline2DGPU::Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2)
{
	std::vector<glm::vec3> transformedNodes = GetTransformedNodes();

	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i] = MathUtils::reflectPoint(nodes[i], linePt1, linePt2);
	}
	for (int i = 0; i < transformedNodes.size(); i++)
	{
		polylineBulgedSamples[i] = MathUtils::reflectPoint(transformedNodes[i], linePt1, linePt2);
	}

	this->centroid = MathUtils::reflectPoint(worldModelMatrix * glm::vec4(this->centroid, 1.0f), linePt1, linePt2);
}

void Polyline2DGPU::SetStartPoint(int index)
{
	if (index != 0)
	{
		std::vector<glm::vec3> temp;
		for (int i = index; i < nodes.size(); i++)
		{
			temp.push_back(nodes[i]);
		}
		for (int i = 0; i < index; i++)
		{
			temp.push_back(nodes[i]);
		}
		nodes = temp;
	}
	GenerateBulgeSamples(nodes, bulges, polylineBulgedSamples);
}

void Polyline2DGPU::Reverse()
{
	std::reverse(nodes.begin(), nodes.end());
	std::reverse(polylineBulgedSamples.begin(), polylineBulgedSamples.end());
	for (auto& bulge : bulges)
	{
		bulge = -bulge;
	}
	std::reverse(bulges.begin(), bulges.end());

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

glm::vec3 Polyline2DGPU::Evaluate(float t)
{
	int index = 0;
	float cumSum = 0;
	float currentSectionDock = 0.0f;;

	while (currentSectionDock < t && (index + 1) < polylineBulgedSamples.size())
	{
		cumSum += glm::distance(polylineBulgedSamples[index], polylineBulgedSamples[index + 1]);
		currentSectionDock = cumSum / pathLength;
		index++;
	}

	glm::vec3 start = polylineBulgedSamples[index - 1];
	glm::vec3 end = polylineBulgedSamples[index];
	glm::vec3 evaluated = start + (end - start) * ((t - (cumSum - glm::distance(start, end)) / pathLength) / (glm::distance(start, end) / pathLength));
	return this->worldModelMatrix * glm::vec4(evaluated, 1.0f);
}

glm::vec3 Polyline2DGPU::Derivative(float t)
{
	int index = 0;
	float cumSum = 0;
	float currentSectionDock = 0.0f;;

	while (currentSectionDock < t && (index + 1) < polylineBulgedSamples.size())
	{
		cumSum += glm::distance(polylineBulgedSamples[index], polylineBulgedSamples[index + 1]);
		currentSectionDock = cumSum / pathLength;
		index++;
	}

	glm::vec3 start = polylineBulgedSamples[index - 1];
	glm::vec3 end = polylineBulgedSamples[index];

	glm::vec3 derivated = glm::vec3(1.0f, (end.y - start.y) / (end.x - start.x), 0.0f);

	return this->worldModelMatrix * glm::vec4(derivated, 1.0f);
}

float Polyline2DGPU::Curvature(float t)
{
	float radius = CurvatureRadius(t);
	return 1.0f / radius;
}

float Polyline2DGPU::CurvatureRadius(float t)
{
	int index = 0;
	float cumSum = 0;
	std::vector<float> samplesT;

	while ((index + 1) < nodes.size())
	{
		samplesT.push_back(cumSum / pathLength);
		cumSum += glm::distance(nodes[index], nodes[(index + 1) % nodes.size()]);
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

	std::tie(center, startAngle, endAngle, radius) = MathUtils::CalculateCircleByThreePoints(nodes[(itIndex - 1)], nodes[itIndex], nodes[(itIndex + 1)]);

	return radius;
}

void Polyline2DGPU::SetParameter(const std::vector<glm::vec3>& nodes, bool isClosed, std::vector<float> bulges)
{
	if (!bulges.size())
	{
		bulges.resize(nodes.size());
		std::fill(bulges.begin(), bulges.end(), 0);
	}
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

	GenerateBulgeSamples(nodes, bulges, polylineBulgedSamples);

	this->bulges = bulges;
	this->nodes = nodes;
	this->isClosed = isClosed;

	if (polylineBulgedSamples.size() <= 2)
	{
		polylineBulgedSamples = nodes;
	}
	bbox = AABB(polylineBulgedSamples[0], polylineBulgedSamples[1]);
	for (int i = 2; i < nodes.size(); i++)
	{
		bbox.Union(nodes[i]);
	}

	centroid = glm::vec3(0.0f);
	boostPath.clear();
	for (const glm::vec3& p : nodes)
	{
		bg::append(boostPath, BoostPoint(p.x, p.y));
		centroid += p;
	}
	if (isClosed)
	{
		this->nodes.push_back(this->nodes[0]);
		bg::append(boostPath, BoostPoint(nodes[0].x, nodes[0].y));
		centroid += nodes[0];
	}
	centroid /= nodes.size();
	area = isClosed ? 0 : MathUtils::ComputeArea(this->GetTransformedNodes());//polyon换成linestring需要自己计算area

	pathLength = 0;
	int size = polylineBulgedSamples.size();
	for (int i = 0; i < size - 1; i++)
	{
		pathLength += glm::distance(polylineBulgedSamples[i], polylineBulgedSamples[i + 1]);
	}
	if (isClosed)
	{
		pathLength += glm::distance(polylineBulgedSamples[size - 1], polylineBulgedSamples[0]);
	}

	int i = 0;
	direction = MathUtils::GetDirection(this->centroid, nodes[0], nodes[1]);
	while (direction == GeomDirection::AtLine && i < nodes.size() - 2)
	{
		i++;
		direction = MathUtils::GetDirection(this->centroid, nodes[i], nodes[i + 1]);
	}
}

float Polyline2DGPU::cumulativeLength(double t0, double t1, double eps)
{
	std::vector<float> samplesT;
	samplesT.push_back(0.0f);
	float cumsum = 0.0f;
	for (int i = 0; i < polylineBulgedSamples.size() - 1; i++)
	{
		cumsum += glm::length(polylineBulgedSamples[i + 1] - polylineBulgedSamples[i]);
		samplesT.push_back(cumsum / pathLength);
	}

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
		res += u * (glm::length(polylineBulgedSamples[iLeft + 1] - polylineBulgedSamples[iLeft]));
		for (int i = iLeft + 1; i < (iRight - 1); i++)
		{
			res += glm::length(polylineBulgedSamples[i + 1] - polylineBulgedSamples[i]);
		}
		u = (t1 - samplesT[iRight - 1]) / (samplesT[iRight] - samplesT[iRight - 1]);
		res += u * (glm::length(polylineBulgedSamples[iRight] - polylineBulgedSamples[iRight - 1]));
	}
	else
	{
		float u = (t1 - t0) / (samplesT[iRight] - samplesT[iLeft]);
		res = u * (glm::length(polylineBulgedSamples[iRight] - polylineBulgedSamples[iLeft]));
	}

	return res;
}

float Polyline2DGPU::findT(const float lastT, const int precision)
{
	float left = lastT;
	float right = 1.0f;
	float target = static_cast<float>(precision);
	float eps = target * 0.001f;
	int maxIter = 50;
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

std::vector<glm::vec3> Polyline2DGPU::GetBulgeSamples(const glm::vec3& node1, const glm::vec3& node2, float bulge)
{
	float b = 0.5f * (1.0f / bulge - bulge);
	float centerX = 0.5f * ((node1.x + node2.x) - b * (node2.y - node1.y));
	float centerY = 0.5f * ((node1.y + node2.y) + b * (node2.x - node1.x));

	return {};
}

void Polyline2DGPU::GenerateBulgeSamples(std::vector<glm::vec3> nodes, const std::vector<float> bulges, std::vector<glm::vec3>& bulgedSamples)
{
	if (bulgedSamples.size())
	{
		bulgeIndexMapper.clear();
		bulgedSamples.clear();
	}

	int index = 0;
	pathLength = 0;
	bool lastReverse = false;
	int size = nodes.size();
	for (const float& bulge : bulges)
	{
		lastReverse = false;
		bulgeIndexMapper[index] = bulgedSamples.size();
		if (bulge != 0)
		{
			glm::vec3 S = nodes[index];
			glm::vec3 E = nodes[(index + 1) % size];
			float startAngle;
			float endAngle;
			glm::vec3 center;

			std::tie(center, startAngle, endAngle) = MathUtils::CalculateArcParamsByBulge(S, E, bulge);
			float radius = glm::distance(S, center);

			Arc2DGPU arc(center, radius, startAngle, endAngle);

			float distanceBegin = glm::distance(*arc.arcSamples.begin(), S);
			if (distanceBegin < 1e-3)
			{
				bulgedSamples.insert(bulgedSamples.end(), arc.arcSamples.begin(), arc.arcSamples.end());
			}
			float distanceEnd = glm::distance(arc.arcSamples.back(), S);
			if (distanceEnd < 1e-3)
			{
				std::reverse(arc.arcSamples.begin(), arc.arcSamples.end());
				bulgedSamples.insert(bulgedSamples.end(), arc.arcSamples.begin(), arc.arcSamples.end());

				lastReverse = true;
			}
			pathLength += arc.pathLength;
		}
		else
		{
			bulgedSamples.push_back(nodes[index]);
			if (index > 1)
			{
				pathLength += glm::distance(nodes[index], nodes[index - 1]);
			}
		}
		index++;
	}
	bulgeIndexMapper[index] = bulgedSamples.size();
	if (!lastReverse && bulges.size() < nodes.size())
	{
		bulgedSamples.push_back(nodes.back());
	}
}

std::vector<glm::vec3> Polyline2DGPU::SplitToSection(float precision)
{
	std::vector<glm::vec3> res;
	res.push_back(this->worldModelMatrix * glm::vec4(nodes[0], 1.0f));

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

	return res;
}

std::vector<glm::vec3> Polyline2DGPU::GetTransformedNodes()
{
	std::vector<glm::vec3> res;
	for (auto& pt : polylineBulgedSamples)
	{
		res.push_back(this->worldModelMatrix * glm::vec4(pt, 1.0f));
	}
	return res;
}

std::string Polyline2DGPU::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);

		glm::vec3 start = transformedMatrix * glm::vec4(nodes[0], 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(nodes[nodes.size() - 1], 1.0f);

		char buffer[256];
		int index = 0;

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

		for (glm::vec3& pos : nodes)
		{
			if (index >= bulges.size() || bulges[index] == 0 || index == nodes.size())
			{
				glm::vec3 transformed = transformedMatrix * glm::vec4(pos, 1.0f);
				std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
				s += buffer;
			}
			else
			{
				glm::vec3 center;
				double startAngle;
				double endAngle;

				glm::vec3 start = transformedMatrix * glm::vec4(nodes[index], 1.0f);
				glm::vec3 end = transformedMatrix * glm::vec4(nodes[(index + 1) % nodes.size()], 1.0f);

				std::tie(center, startAngle, endAngle) = MathUtils::CalculateArcParamsByBulge(start, end, bulges[index]);

				float I = (center - start).x;
				float J = (center - start).y;

				if (bulges[index] < 0) //顺时针
				{
					std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
					s += buffer;
				}
				else
				{
					std::sprintf(buffer, "N%03d G03 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
					s += buffer;
				}
			}
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, bulgeIndexMapper[index] + 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			index++;
		}
		if (isClosed)
		{
			glm::vec3 transformed = transformedMatrix * glm::vec4(nodes[0], 1.0f);
			std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
			s += buffer;
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, 0, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			index++;
		}
		Mstatus->toolPos = end;
	}
	return s;
}

std::string Polyline2DGPU::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string section = "";
	if (createGCode)
	{
		char buffer[256];
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(this->worldModelMatrix, { Mstatus->zoom,Mstatus->zoom ,Mstatus->zoom }, Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);
		glm::vec3 start = transformedMatrix * glm::vec4(nodes[0], 1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(nodes[nodes.size() - 1], 1.0f);

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

		int index = 0;
		for (glm::vec3& pos : nodes)
		{
			if (index >= bulges.size() || bulges[index] == 0 || index == nodes.size())
			{
				glm::vec3 transformed = transformedMatrix * glm::vec4(pos, 1.0f);
				std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
				section += buffer;
			}
			else
			{
				glm::vec3 center;
				double startAngle;
				double endAngle;

				glm::vec3 start = transformedMatrix * glm::vec4(nodes[index], 1.0f);
				glm::vec3 end = transformedMatrix * glm::vec4(nodes[(index + 1) % nodes.size()], 1.0f);

				std::tie(center, startAngle, endAngle) = MathUtils::CalculateArcParamsByBulge(start, end, bulges[index]);

				float I = (center - start).x;
				float J = (center - start).y;

				if (bulges[index] < 0) //顺时针
				{
					std::sprintf(buffer, "N%03d G02 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
					section += buffer;
				}
				else
				{
					std::sprintf(buffer, "N%03d G03 X%f Y%f I%f J%f\n", Mstatus->ncstep++, end.x, end.y, I, J);
					section += buffer;
				}
			}
			if (createRecord)
			{
				GCodeRecord rec(std::string(buffer), this, bulgeIndexMapper[index] + 1, transformedMatrix, Mstatus->ncstep);
				GCodeController::GetController()->AddRecord(rec);
			}
			index++;
		}
		if (isClosed)
		{
			glm::vec3 transformed = transformedMatrix * glm::vec4(nodes[0], 1.0f);
			std::sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
			section += buffer;
			index++;
		}
		Mstatus->toolPos = end;
	}
	return section;
}

QString Polyline2DGPU::Description()
{
	QString description;
	if (editPointIndex < 0)
	{
		glm::vec3 start = worldModelMatrix * glm::vec4(nodes[0], 1.0f);
		glm::vec3 end = worldModelMatrix * glm::vec4(nodes[nodes.size() - 1], 1.0f);

		description = QString("平面多段线,节点数:%1,起点:(%2,%3),终点:(%4,%5) 图形总长: %6")
			.arg(nodes.size()).arg(start.x).arg(start.y).arg(end.x)
			.arg(end.y).arg(pathLength);
	}
	else
	{
		float cumsum = 0;
		int index = 0;
		glm::vec3 editPoint = worldModelMatrix * glm::vec4(nodes[editPointIndex], 1.0f);
		while (index < editPointIndex)
		{
			cumsum += glm::distance(nodes[index], nodes[index + 1]);
			index++;
		}
		float raiuds = CurvatureRadius(static_cast<float>(cumsum) / pathLength);
		QString PointDesc = QString(",编辑点(%1):(%2,%3)").arg(editPointIndex).arg(editPoint.x).arg(editPoint.y);
		description = QString("曲率半径:%1 %2").arg(raiuds).arg(PointDesc);
	}
	return description;
}

void Polyline2DGPU::Simplify(float epsilon)
{
	intermidatePolygon = MathUtils::DouglasPeucker(this->nodes, epsilon);
	UpdatePaintData();
}

void Polyline2DGPU::Smooth(float epsilon)
{
}