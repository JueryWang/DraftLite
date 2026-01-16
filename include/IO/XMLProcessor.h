#pragma once
#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>
#include <QString>

namespace CNCSYS
{
	class SketchGPU;
	class EntityVGPU;
}

struct EntityParseInfo
{
	CNCSYS::EntityVGPU* ent;
	int groupId;
	int ringId;
};

class XMLProcessor
{
public:
	XMLProcessor();
	~XMLProcessor();

	void SaveProject(const QString& filePath);
	void ReadProject(const QString& filePath);

private:
	void SaveScene(QXmlStreamWriter& writer, CNCSYS::SketchGPU* sketch, const QString& TempDir,bool defaultLayer = false);
	std::shared_ptr<CNCSYS::SketchGPU> ParseScene(const QDomElement& sceneElem);
	EntityVGPU* ParseEntity(const QDomElement& entityElem);
};