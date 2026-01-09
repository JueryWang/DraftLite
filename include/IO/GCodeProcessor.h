#pragma once

#include <string>
#include <fstream>
#include "Graphics/Sketch.h"

using namespace CNCSYS;

class GCodeProcessor
{
public:
	GCodeProcessor(CNCSYS::SketchGPU* sketch);
	~GCodeProcessor();
	bool LoadFromFile(const std::string& filePath);
	bool SaveToFile(const std::string& filePath);

private:
	CNCSYS::SketchGPU* mSketch;
};