#pragma once

#include "dxflib/include/dl_dxf.h"
#include "dxflib/include/dl_creationadapter.h"
#include <functional>
#include <memory>
#include <string>

namespace CNCSYS
{
	class SketchGPU;
	class SketchCPU;

	class DXFProcessor
	{
	public:
		DXFProcessor(std::shared_ptr<SketchGPU> sketch);
		DXFProcessor(std::shared_ptr<SketchCPU> sketch);
		void SetCompleteCallback(std::function<void(void)> callback) { onComplete = callback; }
		~DXFProcessor();

		int read(const std::string& dxfFile);
		int write(const std::string& dxfFile);
	private:
		std::weak_ptr<SketchGPU> psketchGPU;
		std::weak_ptr<SketchCPU> psketchCPU;

		std::function<void(void)> onComplete = nullptr;
	};
}