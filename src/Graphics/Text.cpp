#include "Graphics/Text.h"
#include "Common/MathUtils.h"
#include "Common/Program.h"
#include "Graphics/Primitives.h"
#include "UI/GCodeEditor.h"
#include <fstream>
#include <glm/glm.hpp>
#include <algorithm>

FT_Library library;
FT_Face face;

struct Character {
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int font_shift;
};
std::map<unsigned int, Character> Characters;

int FontOutLineMoveTo(const FT_Vector* to, void* user);
int FontOutLineLineTo(const FT_Vector* to, void* user);
int ConicToFunc(const FT_Vector* control, const FT_Vector* to, void* user);
int CubicToFunc(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user);

unsigned int next_utf8_codepoint(const std::string& str, size_t& pos);

Text::Text(FontConfig fconfig) : config(fconfig)
{
	if (FT_Init_FreeType(&library))
	{
		std::cerr << "FreeType 初始化失败" << std::endl;
	}

	if (FT_New_Face(library, fconfig.fontpath.c_str(), 0, &face))
	{
		std::cerr << "无法加载字体:" << std::endl;
		FT_Done_FreeType(library);
	}

	if (FT_Set_Pixel_Sizes(face, 48, 48)) {
		std::cerr << "设置字体尺寸失败" << std::endl;
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}

	int width = 0;
	int max_ascent = 0;
	int max_descent = 0;

	FT_UInt previous = 0;
	for (const char* p = fconfig.content.c_str(); *p != '\0';++p) {
		FT_UInt glyph_index = FT_Get_Char_Index(face, (FT_ULong)*p);

		if (previous && glyph_index) {
			FT_Vector delta = { 0, 0 };
			FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			width += (int)(delta.x >> 6);
		}

		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) == 0) {
			FT_Glyph_Metrics& metrics = face->glyph->metrics;

			width += (int)(metrics.horiAdvance >> 6);

			int ascent = (int)(metrics.horiBearingY >> 6);
			int glyph_height = (int)(metrics.height >> 6);
			int descent = ascent - glyph_height;   // 通常 ≤ 0

			max_ascent = std::max(max_ascent, ascent);
			max_descent = std::max(max_descent, -descent);  // 把负的 descent 转成正的距离
		}
		previous = glyph_index;
	}

	boostPath.clear();
	int height = max_ascent + max_descent;
	float font_shift = 0;
	currentXOffset = 0;
	penPosition = glm::vec3(config.x,config.y,0.0f);
	bbox = AABB(penPosition,penPosition);
	content = config.content;
	std::wstring wstr = QString::fromLocal8Bit(config.content.c_str()).toStdWString();
	LoadText(wstr);
	config.xDimension = currentXOffset - config.spacing;

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

Text::~Text()
{

}

void Text::UpdatePaintData()
{
	if (content != config.content)
	{
		for (EntityVGPU* section : outlinePath)
		{
			delete section;
		}
		//重新解析字形
		LoadText(QString::fromLocal8Bit(config.content.c_str()).toStdWString());
	}
	
	for (int i = 0; i < nodes.size(); i++)
	{
		glm::vec3 transformed = worldModelMatrix * glm::vec4(nodes[i], 1.0f);
		bg::set<0>(boostPath[i], transformed.x);
		bg::set<1>(boostPath[i], transformed.y);
	}

	BoostBox boostBox;
	bg::envelope(boostPath, boostBox);
	BoostPoint min = boostBox.min_corner();
	BoostPoint max = boostBox.max_corner();
	bbox = AABB(glm::vec3(bg::get<0>(min), bg::get<1>(min), 0.0f), glm::vec3(bg::get<0>(max), bg::get<1>(max), 0.0f));
	this->modelMatrixStash = this->worldModelMatrix;
}

glm::vec3 Text::GetStart()
{
	return glm::vec3();
}

glm::vec3 Text::GetEnd()
{
	return glm::vec3();
}

void Text::Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode)
{
	if (isSelected)
	{
		g_lineShader->use();
		g_lineShader->setVec4("PaintColor",glm::vec4(1.0f, 1.0f, 1.0f,1.0f));
		glLineWidth(1.0f);

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);

		glBegin(GL_LINE_LOOP);
		glVertex2f(this->bbox.min.x, this->bbox.min.y);
		glVertex2f(this->bbox.max.x, this->bbox.min.y);
		glVertex2f(this->bbox.max.x, this->bbox.max.y);
		glVertex2f(this->bbox.min.x, this->bbox.max.y);
		glEnd();

		glDisable(GL_LINE_STIPPLE);
	}

	for (EntityVGPU* section : outlinePath)
	{
		section->Paint(g_lineShader,ocsSys,mode);
	}
}

void Text::Move(const glm::vec3& offset)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);
	worldModelMatrix = translation * worldModelMatrix;
	for (EntityVGPU* ent : outlinePath)
	{
		ent->Move(offset);
	}
	this->bbox.min += offset;
	this->bbox.max += offset;
}

void Text::MoveTo(const glm::vec3& pos)
{

}

void Text::Rotate(const glm::vec3& center, float angle)
{

}

void Text::Scale(const glm::vec3& scalar, const glm::vec3& center)
{
}

void Text::Mirror(const glm::vec3 & linePt1, const glm::vec3 & linePt2)
{

}

void Text::SetStartPoint(int index)
{

}

void Text::Reverse()
{

}

glm::vec3 Text::Evaluate(float t)
{
	return glm::vec3();
}

glm::vec3 Text::Derivative(float t)
{
	return glm::vec3();
}

float Text::Curvature(float t)
{
	return 0.0f;
}

float Text::CurvatureRadius(float t)
{
	return 0.0f;
}

std::vector<glm::vec3> Text::GetTransformedNodes()
{
	return std::vector<glm::vec3>();
}

std::vector<glm::vec3> Text::SplitToSection(float precision)
{
	return std::vector<glm::vec3>();
}

std::string Text::ToNcInstruction(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	std::string s = "";
	if (createGCode)
	{
		for (EntityVGPU* section : outlinePath)
		{
			s += section->ToNcInstruction(Mstatus, createRecord, sketch);
		}
	}
	return s;
}

std::string Text::GenNcSection(SimulateStatus* Mstatus, bool createRecord, SketchGPU* sketch)
{
	return std::string();
}

QString Text::Description()
{
	return QString();
}

EntityVGPU* Text::Offset(float value, int precision)
{
	return nullptr;
}

void Text::LoadText(const std::wstring& text)
{
	for (wchar_t codepoint : text)
	{
		LoadCharacter((unsigned int)codepoint);
	}
}

int FontOutLineMoveTo(const FT_Vector* to, void* user)
{
	Text* fontEntity = static_cast<Text*>(user);
	if (fontEntity)
	{
		float finalX = ((float)to->x / 64.0f) + fontEntity->currentXOffset;
		float finalY = (float)to->y / 64.0f;

		char buffer[256];
		sprintf(buffer,"G00 X%.4f Y%.4f\n",finalX,finalY);
		fontEntity->penPosition = glm::vec3(finalX,finalY,0.0);
	}
	return 0;
}

int FontOutLineLineTo(const FT_Vector* to, void* user)
{
	Text* fontEntity = static_cast<Text*>(user);
	if (fontEntity)
	{
		float finalX = ((float)to->x / 64.0f) + fontEntity->currentXOffset;
		float finalY = (float)to->y / 64.0f;

		char buffer[256];
		sprintf(buffer, "G01 X%.4f Y%.4f\n", finalX, finalY);
		Line2DGPU* line = new Line2DGPU({ fontEntity->penPosition,glm::vec3(finalX,finalY,0.0) });
		line->UpdatePaintData();
		fontEntity->outlinePath.push_back(line);
		line->Move(glm::vec3(fontEntity->config.x,fontEntity->config.x,0.0));
		fontEntity->bbox.Union(line->end);
		std::vector<glm::vec3> transformedLine = line->GetTransformedNodes();
		for (glm::vec3& pt : transformedLine)
		{
			bg::append(fontEntity->boostPath, BoostPoint(pt.x, pt.y));
			fontEntity->nodes.push_back(pt);
		}
		fontEntity->penPosition = { finalX, finalY, 0.0 };
	}
	return 0;
}

int FontOutLineConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
	std::vector<glm::vec3> controlPoints;
	Text* fontEntity = static_cast<Text*>(user);
	if (fontEntity)
	{
		controlPoints.push_back(fontEntity->penPosition);
		controlPoints.push_back({ (float)control->x/64.0f + fontEntity->currentXOffset,(float)control->y/64.0f,0.0 });
		controlPoints.push_back({(float)to->x/64.0f + fontEntity->currentXOffset,(float)to->y/64.0f,0.0});
		std::vector<float> knots = {0,0,0,1,1,1};

		glm::vec3 currentPos = fontEntity->penPosition;

		std::vector<glm::vec3> interpPoints;
		for (size_t i = 0; i < 20; i++)
		{
			float t = i / 20.0;
			interpPoints.push_back(MathUtils::CalculateBSpline(controlPoints,knots,2,t));
		}

		for (const glm::vec3& pt : interpPoints)
		{
			char buffer[256];
			sprintf(buffer, "G01 X%.4f Y%.4f\n", pt.x, pt.y);
			Line2DGPU* line = new Line2DGPU({currentPos,glm::vec3(pt.x,pt.y,0.0)});
			line->Move(glm::vec3(fontEntity->config.x, fontEntity->config.x, 0.0));
			line->UpdatePaintData();
			fontEntity->outlinePath.push_back(line);
			fontEntity->bbox.Union(line->end);
			std::vector<glm::vec3> transformedLine = line->GetTransformedNodes();
			for (glm::vec3& pt : transformedLine)
			{
				bg::append(fontEntity->boostPath, BoostPoint(pt.x, pt.y));
				fontEntity->nodes.push_back(pt);
			}
			currentPos = glm::vec3(pt.x,pt.y,0.0);
		}

		Line2DGPU* line = new Line2DGPU({ currentPos,glm::vec3(controlPoints.back().x,controlPoints.back().y,0.0)});
		line->Move(glm::vec3(fontEntity->config.x, fontEntity->config.x, 0.0));
		line->UpdatePaintData();
		fontEntity->outlinePath.push_back(line);
		fontEntity->bbox.Union(line->end);
		std::vector<glm::vec3> transformedLine = line->GetTransformedNodes();
		for (glm::vec3& pt : transformedLine)
		{
			bg::append(fontEntity->boostPath, BoostPoint(pt.x, pt.y));
			fontEntity->nodes.push_back(pt);
		}

		fontEntity->penPosition = { (float)to->x/64.0f + fontEntity->currentXOffset,(float)to->y/64.0f,0.0 };
	}

	return 0;
}

int FontOutLineCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
	std::vector<glm::vec3> controlPoints;
	Text* fontEntity = static_cast<Text*>(user);
	if (fontEntity)
	{
		controlPoints.push_back(fontEntity->penPosition);
		controlPoints.push_back({ (float)control1->x/64.0f + fontEntity->currentXOffset,(float)control1->y/64.0f,0.0 });
		controlPoints.push_back({ (float)control2->x/64.0f + fontEntity->currentXOffset,(float)control2->y/64.0f,0.0 });
		controlPoints.push_back({ (float)to->x/64.0f + fontEntity->currentXOffset,(float)to->y/64.0f,0.0 });
		std::vector<float> knots = { 0,0,0,0,1,1,1,1};

		glm::vec3 currentPos = fontEntity->penPosition;

		std::vector<glm::vec3> interpPoints;
		for (size_t i = 0; i < 20; i++)
		{
			float t = i / 20.0;
			interpPoints.push_back(MathUtils::CalculateBSpline(controlPoints, knots, 3, t));
		}

		for (const glm::vec3& pt : interpPoints)
		{
			char buffer[256];
			sprintf(buffer, "G01 X%.4f Y%.4f\n", pt.x, pt.y);
			Line2DGPU* line = new Line2DGPU({ currentPos,glm::vec3(pt.x,pt.y,0.0) });
			line->Move(glm::vec3(fontEntity->config.x, fontEntity->config.x, 0.0));
			line->UpdatePaintData();
			fontEntity->outlinePath.push_back(line);
			fontEntity->bbox.Union(line->end);
			std::vector<glm::vec3> transformedLine = line->GetTransformedNodes();
			for (glm::vec3& pt : transformedLine)
			{
				bg::append(fontEntity->boostPath, BoostPoint(pt.x, pt.y));
				fontEntity->nodes.push_back(pt);
			}
			currentPos = glm::vec3(pt.x, pt.y, 0.0);
		}

		Line2DGPU* line = new Line2DGPU({ currentPos,glm::vec3(controlPoints.back().x,controlPoints.back().y,0.0) });
		line->Move(glm::vec3(fontEntity->config.x, fontEntity->config.x, 0.0));
		line->UpdatePaintData();
		fontEntity->outlinePath.push_back(line);
		fontEntity->bbox.Union(line->end);
		std::vector<glm::vec3> transformedLine = line->GetTransformedNodes();
		for (glm::vec3& pt : transformedLine)
		{
			bg::append(fontEntity->boostPath, BoostPoint(pt.x, pt.y));
			fontEntity->nodes.push_back(pt);
		}

		fontEntity->penPosition = { (float)to->x/64.0f + fontEntity->currentXOffset,(float)to->y/64.0f,0.0 };
	}

	return 0;
}

void Text::LoadCharacter(unsigned int codepoint)
{
	if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << codepoint << std::endl;
		return;
	}

	if (FT_Load_Char(face, codepoint, FT_LOAD_NO_BITMAP)) return;
	if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) return;

	FT_Outline_Funcs funcs = {};
	funcs.move_to = FontOutLineMoveTo;
	funcs.line_to = FontOutLineLineTo;
	funcs.conic_to = FontOutLineConicTo;
	funcs.cubic_to = FontOutLineCubicTo;

	FT_Outline_Decompose(&face->glyph->outline, &funcs, this);

	FT_Glyph_Metrics& metrics = face->glyph->metrics;
	float advance = (float)(metrics.horiAdvance >> 6);

	currentXOffset += advance;
	currentXOffset += config.spacing;
}
