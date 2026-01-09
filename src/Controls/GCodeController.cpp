#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include "UI/GCodeEditor.h"
#include "Common/ProgressInfo.h"
#include "Graphics/Primitives.h"
#include <QRegularExpression>

enum class GCodeType
{
	G00 = 0,
	G01 = 1,
	G02 = 2,
	G03 = 3,
	G05 = 5
};

struct RecGCode
{
	GCodeType Rtype;
};

struct RecG00 : public RecGCode
{
	float X = -FLT_MAX;
	float Y = -FLT_MAX;
};

struct RecG01 : public RecGCode
{
	float X = -FLT_MAX;
	float Y = -FLT_MAX;
};
struct RecG05 : public RecGCode
{
	float X = -FLT_MAX;
	float Y = -FLT_MAX;
};
struct RecG02 : public RecGCode
{
	float X;
	float Y;
	float I;
	float J;
};
struct RecG03 : public RecGCode
{
	float X;
	float Y;
	float I;
	float J;
};

std::vector<RecGCode*> GRecords;

GCodeController* GCodeController::s_controller = nullptr;

bool endEntity = true;

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
	records.clear();
	lastHighlightEntity = nullptr;
}

void GCodeController::UpdateGCode(const std::vector<EntityVGPU*> entities)
{

}

void GCodeController::ParseGCodeToEntities(const QString& content, std::vector<EntityVGPU*>& entities)
{

	lastHighlightEntity = nullptr;
	lastHighlightPath = nullptr;
	records.clear();

	glm::vec3 curPos = glm::vec3(0.0f);

	QStringList lines = content.split("\n");
	for (const QString& line : lines)
	{
		if (TryParseG00(line))
		{
			continue;
		}
		else if (TryParseG01(line))
		{
			continue;
		}
		else if (TryParseG02(line))
		{
			continue;
		}
		else if (TryParseG03(line))
		{
			continue;
		}
		else if (TryParseG05(line))
		{
			continue;
		}
	}

	for (RecGCode* rec : GRecords)
	{
		delete rec;
	}
	GRecords.clear();

	if (holdSketch)
	{
		holdSketch->UpdateSketch();
	}
}

bool GCodeController::TryParseG00(const QString& line)
{
	static QRegularExpression RegexG00(R"(G00\s+X\s*(-?\d+\.?\d*|\.?\d+)\s+Y\s*(-?\d+\.?\d*|\.?\d+))");
	QRegularExpressionMatchIterator matchItG00 = RegexG00.globalMatch(line);

	while (matchItG00.hasNext())
	{
		QRegularExpressionMatch match = matchItG00.next();
		if (match.hasMatch())
		{
			QString XStr = match.captured(1);
			double XValue = XStr.toDouble();
			QString YStr = match.captured(2);
			double YValue = YStr.toDouble();

			RecG00* stG00 = new RecG00;
			stG00->X = XValue;
			stG00->Y = YValue;
			stG00->Rtype = GCodeType::G00;
			GRecords.push_back(stG00);

			endEntity = !endEntity;
			if (endEntity)
			{
				glm::vec3 curPos = glm::vec3(0.0f);
				for (RecGCode* rec : GRecords)
				{
					switch (rec->Rtype)
					{
						case GCodeType::G00:
						{
							RecG00* rec00 = static_cast<RecG00*>(rec);
							curPos = glm::vec3(rec00->X, rec00->Y, 0.0f);
							break;
						}
						case GCodeType::G01:
						{
							RecG01* rec01 = static_cast<RecG01*>(rec);
							glm::vec3 end = glm::vec3(rec01->X, rec01->Y, 0.0f);
							Line2DGPU* line = new Line2DGPU(curPos, end);
							if (holdSketch)
							{
								holdSketch->AddEntity(line);
							}
							curPos = end;
							break;
						}
						case GCodeType::G02:
						{
							RecG02* rec02 = static_cast<RecG02*>(rec);
							glm::vec3 start = curPos;
							glm::vec3 center = start + glm::vec3(rec02->I, rec02->J, 0.0f);
							glm::vec3 end = glm::vec3(rec02->X, rec02->Y, 0.0f);
							float radius1 = glm::length(start - center);
							float radius2 = glm::length(end - center);
							float startAngle = MathUtils::GetCounterClockwiseAngle(center,glm::vec3(1,0,0), start);
							float endAngle = MathUtils::GetCounterClockwiseAngle(center, glm::vec3(1, 0, 0), end);

							Arc2DGPU* circle = new Arc2DGPU();

							break;
						}
					}
				}
			}
			GRecords.clear();
			return true;
		}
	}

	return false;
}

bool GCodeController::TryParseG01(const QString& line)
{
	static QRegularExpression RegexG01(R"(G01\s+X\s*(-?\d+\.?\d*|\.?\d+)?\s+Y\s*(-?\d+\.?\d*|\.?\d+)?)");
	QRegularExpressionMatchIterator matchItG01 = RegexG01.globalMatch(line);

	while (matchItG01.hasNext())
	{
		QRegularExpressionMatch match = matchItG01.next();
		if (match.hasMatch())
		{
			double XValue = match.captured(1).toDouble();
			double YValue = match.captured(2).toDouble();

			RecG01* stG01 = new RecG01();
			stG01->X = XValue;
			stG01->Y = YValue;
			stG01->Rtype = GCodeType::G01;

			GRecords.push_back(stG01);
		}
	}

	return false;
}

bool GCodeController::TryParseG02(const QString& line)
{
	static QRegularExpression RegexG02(
		R"(G02(?:\s+(?:X(-?\d+\.?\d*|\.?\d+)|Y(-?\d+\.?\d*|\.?\d+)|I(-?\d+\.?\d*|\.?\d+)|J(-?\d+\.?\d*|\.?\d+)))*)",
		QRegularExpression::CaseInsensitiveOption
	);
	QRegularExpressionMatchIterator matchItG02 = RegexG02.globalMatch(line);

	while (matchItG02.hasNext())
	{
		QRegularExpressionMatch match = matchItG02.next();
		if (match.hasMatch())
		{
			double XValue = match.captured(1).toDouble();
			double YValue = match.captured(2).toDouble();
			double IValue = match.captured(3).toDouble();
			double JValue = match.captured(4).toDouble();

			RecG02* stG02 = new RecG02();
			stG02->X = XValue;
			stG02->Y = YValue;
			stG02->I = IValue;
			stG02->J = JValue;
			stG02->Rtype = GCodeType::G02;

			GRecords.push_back(stG02);
			return true;
		}
	}

	return false;
}

bool GCodeController::TryParseG03(const QString& line)
{
	static QRegularExpression RegexG03(R"(G03\s+X\s*(-?\d+\.?\d*|\.?\d+)\s+Y\s*(-?\d+\.?\d*|\.?\d+)\s+
											 I\s+(-?\d+\.?\d*|\.?\d+)\s+J(-?\d+\.?\d*|\.?\d+))");
	QRegularExpressionMatchIterator matchItG03 = RegexG03.globalMatch(line);

	while (matchItG03.hasNext())
	{
		QRegularExpressionMatch match = matchItG03.next();
		if (match.hasMatch())
		{
			double XValue = match.captured(1).toDouble();
			double YValue = match.captured(2).toDouble();
			double IValue = match.captured(3).toDouble();
			double JValue = match.captured(4).toDouble();

			RecG03* stG03 = new RecG03();
			stG03->X = XValue;
			stG03->Y = YValue;
			stG03->I = IValue;
			stG03->J = JValue;
			stG03->Rtype = GCodeType::G03;

			GRecords.push_back(stG03);
			return true;
		}
	}

	return false;
}

bool GCodeController::TryParseG05(const QString& line)
{
	static QRegularExpression RegexG05(R"(G05\s+X\s*(-?\d+\.?\d*|\.?\d+)\s+Y\s*(-?\d+\.?\d*|\.?\d+))");
	QRegularExpressionMatchIterator matchItG05 = RegexG05.globalMatch(line);

	while (matchItG05.hasNext())
	{
		QRegularExpressionMatch match = matchItG05.next();
		if (match.hasMatch())
		{
			double XValue = match.captured(1).toDouble();
			double YValue = match.captured(2).toDouble();

			RecG05* stG05 = new RecG05();
			stG05->X = XValue;
			stG05->Y = YValue;
			stG05->Rtype = GCodeType::G05;
			GRecords.push_back(stG05);
		}
	}

	return false;
}

GCodeController::GCodeController()
{
}

GCodeController::~GCodeController()
{
}
