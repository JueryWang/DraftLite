#pragma once
#include "Graphics/DrawEntity.h"
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

typedef std::vector<glm::vec3> CharacterNodes;

enum FontMode
{
	TrueType = 0,
	SHX,
	PointArray
};

struct FontConfig
{
	double x;					//初始x位置
	double y;					//初始y位置
	double xDimension;			//字号
	double yDimension;			//字体高度
	std::string fontpath;		//字体文件路径
	std::string content;		//文本内容
	int spacing;				//字间距
	int linespacing;			//行间距
	FontMode mode;				//字形				
};

using namespace CNCSYS;

class Text : public EntityVGPU
{
public:
	Text(FontConfig fconfig);
	~Text();
	void ClearFonts();
	void SetConfig(FontConfig fconfig) { this->config = fconfig;}
	virtual EntityType GetType() const { return EntityType::Text; }
	virtual void UpdatePaintData() override;
	virtual glm::vec3 GetStart() override;
	virtual glm::vec3 GetEnd() override;
	virtual void Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode) override;
	virtual void Move(const glm::vec3& offset) override;
	virtual void MoveTo(const glm::vec3& pos) override;
	virtual void Rotate(const glm::vec3& center, float angle) override;
	virtual void Scale(const glm::vec3& scalar, const glm::vec3& center) override;
	virtual void Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2) override;
	virtual void SetStartPoint(int index) override;
	virtual void Reverse() override;
	virtual glm::vec3 Evaluate(float t) override;
	virtual glm::vec3 Derivative(float t) override;
	virtual glm::vec3 Tangent(float t) { return glm::normalize(this->Derivative(t)); };
	virtual glm::vec3 Normal(float t) { glm::vec3 tan = this->Tangent(t); return glm::vec3(-tan.y, tan.x, 0.0f); };
	virtual float Curvature(float t) override;
	virtual float CurvatureRadius(float t) override;
	virtual std::vector<glm::vec3> GetTransformedNodes() override;
	virtual glm::vec3 GetTransformedCentroid() { return worldModelMatrix * glm::vec4(this->centroid, 1.0f); }
	virtual std::vector<glm::vec3> SplitToSection(float precision) override;
	//带G00移动到起始点
	virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
	//不带G00移动到起始点
	virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
	virtual QString Description() override;
	virtual EntityVGPU* Offset(float value, int precision);

private:
	void CreateText(FontConfig config);
	void LoadCharacter(unsigned int codepoint);
	void LoadText(const std::wstring& text);
public:
	FontConfig config;

	std::vector<EntityVGPU*> outlinePath;
	glm::vec3 penPosition;
	float currentXOffset;
	std::string	content;
	std::vector<glm::vec3> nodes;
};