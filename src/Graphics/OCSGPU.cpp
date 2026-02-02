#include "Graphics/OCS.h"
#include "Graphics/AABB.h"
#include "Common/MathUtils.h"
#include "Graphics/Sketch.h"
#include <cmath>

#define TICKER_REMAIN_SPACE 0.08

namespace CNCSYS
{
	OCSGPU::OCSGPU(std::shared_ptr<SketchGPU> sketch) : sketch(sketch)
	{
		sketch.get()->attachedOCS = this;
	}
	OCSGPU::~OCSGPU()
	{
		sketch.reset();
		delete camera;
		delete objectRange;
		delete canvasRange;
	}

	void OCSGPU::SetSketch(std::shared_ptr<SketchGPU> SK)
	{ 
		sketch = SK; 
		SK.get()->attachedOCS = this; 
	}

	void OCSGPU::SetCanvasSizae(int width, int height)
	{
		canvasWidth = width;
		canvasHeight = height;
	}

	void OCSGPU::ComputeScaleFitToCanvas()
	{
		if (sketch && sketch->entities.size() > 0)
		{
			objectRange = new AABB((sketch->entities[0]->bbox));

			for (int i = 1; i < sketch->entities.size(); i++)
			{
				objectRange->Union(&sketch->entities[i]->bbox);
			}

			glm::vec3 canvasCenter = objectRange->Center();
			glm::vec3 canvasLB;
			glm::vec3 canvasRT;
			float widthExpand = objectRange->XRange() / canvasWidth;
			float heightExpand = objectRange->YRange() / canvasHeight;
			float maxExpand = max(widthExpand, heightExpand);

			canvasLB = glm::vec3(canvasCenter.x - (maxExpand * canvasWidth) / 2 * 1 / fitRatio, canvasCenter.y - (maxExpand * canvasHeight) / 2 * 1 / fitRatio, 0.0f);
			canvasRT = glm::vec3(canvasCenter.x + (maxExpand * canvasWidth) / 2 * 1 / fitRatio, canvasCenter.y + (maxExpand * canvasHeight) / 2 * 1 / fitRatio, 0.0f);

			canvasLB *= scale;
			canvasRT *= scale;
			canvasRange = new AABB(canvasLB, canvasRT);

			glm::vec3 centerCanvas = canvasRange->Center();
			glm::vec3 centerObj = objectRange->Center();
			canvasRange->Translate(centerObj - centerCanvas);
		}
		else
		{
			objectRange = new AABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(canvasWidth * fitRatio * scale, canvasHeight * fitRatio * scale, 0.0f));
			glm::vec3 canvasCenter = objectRange->Center();
			glm::vec3 canvasLB = glm::vec3(canvasCenter.x - objectRange->XRange() / 2 * 1 / fitRatio, canvasCenter.x - objectRange->YRange() / 2 * 1 / fitRatio, 0.0f);
			glm::vec3 canvasRT = glm::vec3(canvasCenter.x + objectRange->XRange() / 2 * 1 / fitRatio, canvasCenter.y + objectRange->YRange() / 2 * 1 / fitRatio, 0.0f);
			canvasLB *= scale;
			canvasRT *= scale;
			canvasRange = new AABB(canvasLB, canvasRT);
		}

		if (camera == nullptr)
		{
			camera = new Camera2D(canvasRange);
		}
		else
		{
			camera->UpdateRange(canvasRange);
		}
	}
	void OCSGPU::FitToZero()
	{
		glm::vec3 translation = objectRange->getMin();
		canvasRange->Translate(-translation);
		objectRange->Translate(-translation);
		translationToZero = -translation;
		camera->UpdateRange(canvasRange);
	}
	void OCSGPU::SetFitRatio(float ratio)
	{
		fitRatio = ratio;
	}
	void OCSGPU::UpdateTickers()
	{
		tickers.clear();

		float xrange = canvasRange->XRange();
		float yrange = canvasRange->YRange();
		int xpower = 0;
		int ypower = 0;

		int xdigit = MathUtils::GetFirstNoneZeroDigit(xrange / 25.0f, xpower);
		int ydigit = MathUtils::GetFirstNoneZeroDigit(yrange / 25.0f, ypower);

		if (xdigit > 8)
			XTickUnit = 8 * pow(10, xpower);
		else if (xdigit > 6)
			XTickUnit = 6 * pow(10, xpower);
		else if (xdigit > 4)
			XTickUnit = 4 * pow(10, xpower);
		else if (xdigit > 2)
			XTickUnit = 2 * pow(10, xpower);
		else
			XTickUnit = 1 * pow(10, xpower);

		if (ydigit > 8)
			YTickUnit = 8 * pow(10, xpower);
		else if (xdigit > 6)
			YTickUnit = 6 * pow(10, xpower);
		else if (xdigit > 4)
			YTickUnit = 4 * pow(10, xpower);
		else if (xdigit > 2)
			YTickUnit = 2 * pow(10, xpower);
		else
			YTickUnit = 1 * pow(10, xpower);

		float divisorX = (5 * XTickUnit);
		float divisorY = (5 * YTickUnit);

		float beginX = canvasRange->min.x;
		float beginY = canvasRange->min.y;
		float endX = canvasRange->max.x;
		float endY = canvasRange->max.y;

		beginX = beginX + fmod(abs(beginX), XTickUnit);
		beginX = MathUtils::SmallestCeilling(beginX, xpower);

		while (beginX < endX)
		{
			float remainder = fmod(beginX, divisorX);
			bool isMainAxis = abs(remainder) < XTickUnit;
			glm::vec2 lineStart = glm::vec2(2 * (beginX - canvasRange->min.x) / canvasRange->XRange() - 1.0f, -1.0f);
			glm::vec2 lineEnd = glm::vec2(lineStart.x, isMainAxis ? -0.95f : -0.98f);
			std::vector<glm::vec2> tickLine = { lineStart,lineEnd };
			AxisTicker tick = AxisTicker(beginX, tickLine, isMainAxis ? TickType::Main : TickType::Sub, AxisType::X);
			tickers.push_back(tick);
			beginX += XTickUnit;
		}

		beginY = beginY + fmod(abs(beginY), YTickUnit);
		beginY = MathUtils::SmallestCeilling(beginY, ypower);

		while (beginY < endY)
		{
			float remainder = fmod(beginY, divisorY);
			bool isMainAxis = abs(remainder) < YTickUnit;
			glm::vec2 lineStart = glm::vec2(-1.0f, 2 * (beginY - canvasRange->min.y) / canvasRange->YRange() - 1.0f);
			glm::vec2 lineEnd = glm::vec2(isMainAxis ? -0.95f : -0.98f, lineStart.y);
			std::vector<glm::vec2> tickLine = { lineStart,lineEnd };
			AxisTicker tick = AxisTicker(beginY, tickLine, isMainAxis ? TickType::Main : TickType::Sub, AxisType::Y);
			tickers.push_back(tick);
			beginY += YTickUnit;
		}
	}

	void OCSGPU::OnMouseScroll(float delta, const glm::vec2& mousePosition)
	{
		glm::vec2 preOcsPos = GetOCSPosWithPixelPos(mousePosition);
		float scalar = delta > 0 ? zoomFactor : 1.0f / zoomFactor;
		scale *= scalar;
		glm::vec3 offset = glm::vec3(preOcsPos, 0.0f) - canvasRange->Center();
		canvasRange->Translate(offset);
		canvasRange->Multiply(scalar);
		canvasRange->Translate(-offset * scalar);
		camera->UpdateRange(canvasRange);

		UpdateTickers();
	}

	void OCSGPU::ScrollByCenter(float delta)
	{
		glm::vec2 center = glm::vec2(canvasRange->Center().x, canvasRange->Center().y);
		float scalar = delta > 0 ? (1.0 + fabs(delta)) : 1.0f / (1.0 + fabs(delta));
		canvasRange->Multiply(scalar);
		camera->UpdateRange(canvasRange);

		UpdateTickers();
	}

	void OCSGPU::OnMouseMove(const glm::vec2& offset)
	{
		glm::vec3 canvasOffset;
		canvasOffset.x = offset.x / canvasWidth * canvasRange->XRange();
		canvasOffset.y = offset.y / canvasHeight * canvasRange->YRange();
		canvasRange->Translate(canvasOffset);
		glm::vec3 center = canvasRange->Center();

		camera->UpdateRange(canvasRange);

		UpdateTickers();
	}

	glm::vec3 OCSGPU::GetOCSPosWithPixelPos(const glm::vec2& pixelPos)
	{
		float mouseXfraction = pixelPos.x / canvasWidth;
		float mouseYfraction = 1 - pixelPos.y / canvasHeight;

		return glm::vec3(canvasRange->min.x + mouseXfraction * canvasRange->XRange(), canvasRange->min.y + mouseYfraction * canvasRange->YRange(), 0.0f);
	}

	glm::vec2 OCSGPU::GetPixelPosWithOCSPos(const glm::vec3& ocsPos)
	{
		float xFactor = (ocsPos.x - canvasRange->getMin().x)/(canvasRange->XRange());
		float yFactor = (ocsPos.y - canvasRange->getMin().y)/(canvasRange->YRange());

		return glm::vec2(canvasWidth * xFactor,canvasHeight * (1.0 - yFactor));
	}

	glm::vec3 OCSGPU::OffsetFromScreenToCanvas(const glm::vec3& offset)
	{
		float Xfraction = offset.x / canvasWidth;
		float Yfraction = offset.y / canvasHeight;

		return glm::vec3(Xfraction * canvasRange->XRange(), Yfraction * canvasRange->YRange(), 0.0f);
	}


}
