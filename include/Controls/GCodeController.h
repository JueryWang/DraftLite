#pragma once
#include <QString>
#include "Common/Program.h"
#include "Graphics/DrawEntity.h"
#include "glm/glm.hpp"
#include "Path/Path.h"
#include <vector>

namespace CNCSYS
{
	class SketchGPU;
}

using namespace CNCSYS;

class GCodeController
{
	friend class GCodeEditor;
public:
	static GCodeController* GetController();

	void AddRecord(GCodeRecord& rec);
	void CleanCache();
	void SetSketch(SketchGPU* _sketch) { holdSketch = _sketch; }
private:
	GCodeController();
	~GCodeController();

private:
	static GCodeController* s_controller;
	std::vector<GCodeRecord> records;
	EntityVGPU* lastHighlightEntity = nullptr;
	Path2D* lastHighlightPath = nullptr;
	CNCSYS::SketchGPU* holdSketch;

	glm::mat4 baseTransform;
};