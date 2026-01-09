#include <algorithm>
#include "Graphics/Sketch.h"
#include "Common/MathUtils.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Canvas.h"

namespace CNCSYS
{
	SketchCPU::SketchCPU()
	{

	}

	SketchCPU::~SketchCPU()
	{
	}

	void SketchCPU::AddEntity(EntityVCPU* e)
	{
		entities.push_back(e);
	}

	void SketchCPU::EraseEntity(EntityVCPU* e)
	{
		auto find = std::find(entities.begin(), entities.end(), e);
		if (find != entities.end())
		{
			entities.erase(find);
		}
	}
	void SketchCPU::ClearEntities()
	{
		for (EntityVCPU* ent : entities)
		{
			delete ent;
		}
		entities.clear();
		parts.clear();
		partPreview.clear();
		envolopProfile.clear();
	}

	void SketchCPU::UpdateSketch()
	{
		mainCanvas->UpdateOCS();
	}

	void SketchCPU::SplitPart()
	{
	}

	void SketchCPU::SimplifyGeometry(float delta)
	{
	}

	void SketchCPU::SmoothGeometry()
	{
	}

	void SketchCPU::GenEnvolop(int expandValue, int smoothValue)
	{
	}

	void SketchCPU::ToNcProgram(const std::string& file)
	{

	}
}