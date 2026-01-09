#pragma once
#include <QString>
#include "Common/ProgressInfo.h"
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
	void UpdateGCode(const std::vector<EntityVGPU*> entities);
	void ParseGCodeToEntities(const QString& content, std::vector<EntityVGPU*>& entities);
	void SetSketch(SketchGPU* _sketch) { holdSketch = _sketch; }
private:
	GCodeController();
	~GCodeController();
	bool TryParseG00(const QString& line);
	bool TryParseG01(const QString& line);
	bool TryParseG02(const QString& line);
	bool TryParseG03(const QString& line);
	bool TryParseG05(const QString& line);

private:
	static GCodeController* s_controller;
	std::vector<GCodeRecord> records;
	EntityVGPU* lastHighlightEntity = nullptr;
	Path2D* lastHighlightPath = nullptr;
	CNCSYS::SketchGPU* holdSketch;

	glm::mat4 baseTransform;
};