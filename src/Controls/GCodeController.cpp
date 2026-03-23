#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include "UI/GCodeEditor.h"
#include "Common/Program.h"
#include "Graphics/Primitives.h"
#include <QRegularExpression>

GCodeController* GCodeController::s_controller = nullptr;

GCodeController* GCodeController::GetController()
{
	if (s_controller == nullptr)
	{
		s_controller = new GCodeController();
	}
	return s_controller;
}

void GCodeController::AddRecord(GCodeRecord& rec)
{
	records.push_back(rec);
}

void GCodeController::CleanCache()
{
	for (GCodeRecord& rec : records)
	{
		if (rec.attachedEntity != nullptr)
		{
			rec.attachedEntity->SetHighLight(false);
		}
	}
	records.clear();
	lastHighlightEntity = nullptr;
}

GCodeController::GCodeController()
{
}

GCodeController::~GCodeController()
{
}
