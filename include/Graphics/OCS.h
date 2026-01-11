#pragma once
#include "Graphics/DrawEntity.h"
#include "Graphics/AxisTicker.h"
#include "Graphics/Camera2D.h"
#include <vector>

namespace CNCSYS
{
	class AABB;
	class SketchGPU;
	class SketchCPU;

	class OCSGPU
	{
	public:
		OCSGPU(std::shared_ptr<SketchGPU> SK);
		~OCSGPU();

		void SetSketch(std::shared_ptr<SketchGPU> SK) { sketch = SK; }
		void SetCanvasSizae(int width, int height);
		void ComputeScaleFitToCanvas();
		void FitToZero();
		void SetFitRatio(float ratio);
		void UpdateTickers();
		void OnMouseScroll(float delta, const glm::vec2& mousePosition);
		void OnMouseMove(const glm::vec2& offset);
		void ScrollByCenter(float delta);
		Camera2D* GetCamera() { return camera; }

		//根据屏幕像素坐标获取画布坐标
		inline glm::vec3 GetOCSPosWithPixelPos(const glm::vec2& pixelPos);
		glm::vec3 OffsetFromScreenToCanvas(const glm::vec3& offset);

	public:
		int canvasWidth;
		int canvasHeight;
		std::vector<AxisTicker> tickers;

		std::shared_ptr<SketchGPU> sketch;
		AABB* objectRange = nullptr;
		AABB* canvasRange = nullptr;
		Camera2D* camera = nullptr;

		glm::vec3 translationToZero;

		float scale = 1.0f;
		float zoomFactor = 1.149f;
		float fitRatio = 0.5f;
		float XOffset;
		float YOffset;
		float XTickUnit;
		float YTickUnit;
		bool genTickers = false;
	};

	class OCSCPU
	{
	public:
		OCSCPU(std::shared_ptr<SketchCPU> SK);
		~OCSCPU();

		void SetCanvasSizae(int width, int height);
		void ComputeScaleFitToCanvas();
		void UpdateTickers();
		void OnMouseScroll(float delta, const glm::vec2& mousePosition);
		void OnMouseMove(const glm::vec2& offset);
		Camera2D* GetCamera() { return camera; }

		glm::vec3 GetOCSPosWithPixelPos(const glm::vec2& pixelPos);

	public:
		int canvasWidth;
		int canvasHeight;
		std::vector<AxisTicker> tickers;

		std::shared_ptr<SketchCPU> sketch;
		AABB* objectRange = nullptr;
		AABB* canvasRange = nullptr;
		Camera2D* camera = nullptr;

		float scale = 1.0f;
		float zoomFactor = 1.149f;
		float XOffset;
		float YOffset;
		float XTickUnit;
		float YTickUnit;
		bool genTickers = false;
	};
}