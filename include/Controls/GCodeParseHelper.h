#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include <iostream>
#include <string>
#include <regex>
#include <vector>

struct GCodeLine {
	int line = 0;
	enum GType
	{
		G00,
		G01,
		G02,
		G03,
		G05
	};
	GType type;
	double x, y;
	double i, j;
};

using namespace CNCSYS;

class GCodeParseHelper
{
public:
	GCodeParseHelper(SketchGPU* sketch);
	~GCodeParseHelper();

	void ParseFileToSketch(const std::string fileName);
private:
	SketchGPU* holdSketch = nullptr;
	std::vector< GCodeLine> glines;
};
