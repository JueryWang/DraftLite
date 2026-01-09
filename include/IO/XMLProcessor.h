#pragma once
#include <QFile>
#include <QXmlStreamWriter>
#include <QString>

namespace CNCSYS
{
	class SketchGPU;
}

class XMLProcessor
{
public:
	XMLProcessor();
	~XMLProcessor();

	void SaveProject(const QString& filePath);
	void ReadProject(const QString& filePath);

private:
	void SaveScene(QXmlStreamWriter& writer, CNCSYS::SketchGPU* sketch, const QString& TempDir);
	void ParseScene(QXmlStreamReader& reader, const QString& projPath);
};