#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include "IO/XMLProcessor.h"
#include "IO/ObjectSerializer.h"
#include "UI/TaskFlowGuide.h"
#include "UI/TaskListWindow.h"
#include "UI/MainLayer.h"
#include "Common/Program.h"
#include "Common/CraftParamConfig.h"
#include "Controls/GlobalPLCVars.h"
#include "NetWork/MessageValidtor.h"
#include "UI/TaskListWindow.h"
#include "IO/DxfProcessor.h"
#include "Algorithm/RingDetector.h"
#include "Algorithm/PartClassifier.h"
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <IO/Utils.h>
#include <QUUID>
#include <QProcess>
#include <QApplication>
#include <QMap>
#include <algorithm>

using namespace CNCSYS;
/*
* tar -cvf - "zlib-develop" | pigz > zlib.proj
*/
XMLProcessor::XMLProcessor()
{

}

XMLProcessor::~XMLProcessor()
{

}

void XMLProcessor::SaveProject(const QString& Filename)
{
	QString tempPath = QApplication::applicationDirPath() + "/temp/" + SplitFileFromPath(Filename);
	QDir tempDir(tempPath);
	if (!tempDir.exists())
	{
		tempDir.mkpath(tempPath);
	}

	QString xmlFile = tempPath + "/" + SplitFileFromPath(Filename);
	QFile file(xmlFile);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}

	QXmlStreamWriter writer(&file);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	{
		writer.writeStartElement("Project");
		std::vector<SketchGPU*> sketchs = TaskListWindow::GetInstance()->GetAllTaskSketches();

		{
			writer.writeStartElement("Layers");
			for (SketchGPU* sketch : sketchs)
			{
				SaveScene(writer, sketch, tempPath);
			}

			//写入默认画布的草图配置
			writer.writeStartElement("Scene");
			SketchGPU* sketch = g_mainWindow->GetSketch();
			std::vector<EntityVGPU*> entities = sketch->GetEntities();
			if (sketch->source.size())
			{
				QString filename = SplitFileFromPath(QString::fromLocal8Bit(sketch->source.c_str()));
				writer.writeAttribute("Source", filename);
				QFile::copy(QString::fromLocal8Bit(sketch->source.c_str()), tempPath + "/" + filename);
			}

			writer.writeAttribute("id", QString("Default Layer"));
			SaveScene(writer, sketch, tempPath, true);
			writer.writeEndElement();

			writer.writeEndElement();
		}

		{
			writer.writeStartElement("PLC");
			writer.writeTextElement("URL", g_plcUrl);
			writer.writeTextElement("ParseVarRoot", g_plcSearchRootNode);
			writer.writeTextElement("VarConfig", g_plcVarCnfigExcelUrl);
			QString fileName = SplitFileFromPath(g_plcVarCnfigExcelUrl);
			QFile::copy(g_plcVarCnfigExcelUrl, tempPath + "/" + fileName);
			writer.writeEndElement();
		}

		{

			writer.writeStartElement("ConfigableKeys");
			for (auto& pair : g_ConfigableKeys)
			{
				writer.writeStartElement("Item");
				writer.writeAttribute("key", QString::fromStdString(pair.first));
				writer.writeTextElement("address", QString::fromStdString(pair.second));
				writer.writeEndElement();
			}
			writer.writeEndElement();
		}

		writer.writeEndElement();
	}

	writer.writeEndDocument();
	file.close();

	CompressProject(tempPath, Filename);
	tempDir.removeRecursively();
}
//
void XMLProcessor::ReadProject(const QString& FileName)
{
	QString tempDir = QApplication::applicationDirPath() + "/temp";
	if (DecompressProject(FileName, tempDir))
	{
		QString pureName = SplitFileFromPath(FileName);
		QString conpressedPath = tempDir + "/" + pureName;
		TaskListWindow::GetInstance()->ClearItems();

		QFile file(conpressedPath + "/" + pureName);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			return;
		}

		QDomDocument doc;
		QString errorMsg;
		int errorLine, errorCol;
		// 加载 XML 到 DomDocument（处理编码，这里是 UTF-8）
		if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
			qDebug() << "XML Parse Error - Line:" << errorLine << ", Col:" << errorCol << ":" << errorMsg;
			file.close();
			return;
		}
		file.close();

		QDomElement rootElem = doc.documentElement();
		if (rootElem.tagName() != "Project") {
			qDebug() << "Invalid XML root node (expected 'Project')";
			return;
		}

		QDomElement layersElem = rootElem.firstChildElement("Layers");
		if (!layersElem.isNull()) {
			// 遍历 Layers 下的所有 Scene 节点
			QDomElement sceneElem = layersElem.firstChildElement("Scene");
			while (!sceneElem.isNull()) {
				ParseScene(sceneElem);
				sceneElem = sceneElem.nextSiblingElement("Scene"); // 下一个同级别 Scene
			}
		}
	}
}


void XMLProcessor::SaveScene(QXmlStreamWriter& writer, CNCSYS::SketchGPU* sketch, const QString& TempDir, bool defaultLayer)
{
	if (!defaultLayer)
	{
		writer.writeStartElement("Scene");
		std::vector<EntityVGPU*> entities = sketch->GetEntities();
		if (sketch->source.size())
		{
			QString filename = SplitFileFromPath(QString::fromLocal8Bit(sketch->source.c_str()));
			writer.writeAttribute("Source", filename);
			QFile::copy(QString::fromLocal8Bit(sketch->source.c_str()), TempDir + "/" + filename);
		}
		std::vector<SketchGPU*> taskSketches = TaskListWindow::GetInstance()->GetAllTaskSketches();
		bool findInTask = false;
		for (int i = 0; i < taskSketches.size(); i++)
		{
			if (taskSketches[i] == sketch)
			{
				writer.writeAttribute("id", QString("Layer#%1").arg(i));
				findInTask = true;
				break;
			}
		}
		if (!findInTask)
		{
			writer.writeAttribute("id", QString("Default Layer"));
		}
	}

	auto groups = sketch->GetEntityGroups();

	int groupId = 0;
	for (EntGroup* group : groups)
	{
		int ringId = 0;
		for (EntRingConnection* ring : group->rings)
		{
			for (EntityVGPU* ent : ring->conponents)
			{
				writer.writeStartElement("Entity");
				writer.writeAttribute("GroupId", QString::number(groupId));
				writer.writeAttribute("RingId", QString::number(ringId));
				writer.writeTextElement("type", QString::number(static_cast<int>(ent->GetType())));
				std::string serilize = serilize_to_string(ent);
				writer.writeTextElement("content", QString::fromStdString(serilize));
				writer.writeEndElement();
			}
			ringId++;
		}
		groupId++;
	}

	for (CraftParamConfig& config : sketch->attachedConfig)
	{
		writer.writeStartElement("CraftParams");
		writer.writeTextElement("alias", config.alias);
		writer.writeTextElement("type", QString::number(static_cast<int>(config.plcInfo->dataType)));
		writer.writeTextElement("address", QString::fromLocal8Bit(config.plcInfo->identifier));
		switch (config.plcInfo->dataType)
		{
		case AtomicVarType::BOOL:
		{
			bool value = std::get<bool>(config.preSetVal);
			writer.writeTextElement("value", value ? "True" : "False");
			break;
		}
		case AtomicVarType::WORD:
		{
			uint16_t value = std::get<uint16_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::DWORD:
		{
			uint32_t value = std::get<uint32_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::LWORD:
		{
			uint64_t value = std::get<uint64_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::INT:
		{
			int16_t value = std::get<int16_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::DINT:
		{
			int32_t value = std::get<int32_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::LINT:
		{
			int64_t value = std::get<int64_t>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::REAL:
		{
			float value = std::get<float>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::LREAL:
		{
			double value = std::get<double>(config.preSetVal);
			writer.writeTextElement("value", QString::number(value));
			break;
		}
		case AtomicVarType::STRING:
		{
			std::string value = std::get<std::string>(config.preSetVal);
			writer.writeTextElement("value", QString::fromStdString(value));
			break;
		}
		}
		writer.writeTextElement("remark", config.remark);
		writer.writeEndElement();
	}
	if (!defaultLayer)
	{
		writer.writeEndElement();
	}

}

std::shared_ptr<CNCSYS::SketchGPU> XMLProcessor::ParseScene(const QDomElement& sceneElem)
{
	QString fileSource = sceneElem.attribute("Source");
	QString layerId = sceneElem.attribute("id");

	std::shared_ptr<CNCSYS::SketchGPU> createdSketch;
	if (layerId != "Default Layer")
	{
		createdSketch.reset(new SketchGPU());
	}
	else
	{
		createdSketch = g_canvasInstance->GetSketchShared();
		createdSketch->ClearEntities();
	}

	createdSketch->source = fileSource.toLocal8Bit();
	std::vector< EntityVGPU*> parsedEntities;
	// 遍历 Scene 下的所有 Entity 节点
	QDomElement entityElem = sceneElem.firstChildElement("Entity");
	while (!entityElem.isNull()) {
		parsedEntities.push_back(ParseEntity(entityElem));
		entityElem = entityElem.nextSiblingElement("Entity"); // 下一个同级别 Entity
	}

	std::vector<EntRingConnection*> rings = RingDetector::RingDetect(parsedEntities);
	PartClassifier classifier(rings);
	std::vector<EntGroup*> parsedGroups = classifier.Execute();


	for (EntGroup* group : parsedGroups)
	{
		for (EntRingConnection* ring : group->rings)
		{
			ring->RepairStart();

			if (ring->direction == GeomDirection::CW)
			{
				ring->Reverse();
				ring->direction = GeomDirection::CCW;
				ring->RepairStart();
			}
		}
		createdSketch->AddEntityGroup(group);
	}

	auto groups = parsedGroups;
	if (layerId != "Default Layer")
	{
		ToDoListItem* itemNew = new ToDoListItem();
		ToDoListItemWidget* itemWidget = new ToDoListItemWidget(createdSketch, TaskListWindow::GetInstance());
		TaskListWindow::GetInstance()->AddTaskItem(itemNew, itemWidget);
	}
	else
	{
		g_canvasInstance->SetScene(createdSketch, createdSketch->attachedOCS);
	}

	return createdSketch;
}


EntityVGPU* XMLProcessor::ParseEntity(const QDomElement& entityElem)
{
	EntityVGPU* ent;
	int groupId = entityElem.attribute("GroupId").toInt();
	int ringId = entityElem.attribute("RingId").toInt();
	int type = entityElem.firstChildElement("type").text().toInt();
	std::string content = entityElem.firstChildElement("content").text().toStdString();
	EntityType EntType = static_cast<EntityType>(type);
	switch (EntType)
	{
	case EntityType::Point:
	{
		Point2DGPU* newPoint = new Point2DGPU();
		Point2DGPU revoked_point = deserilize_from_string<Point2DGPU>(content);
		newPoint->Copy(&revoked_point);
		ent = newPoint;
		break;
	}
	case EntityType::Line:
	{
		Line2DGPU* newLine = new Line2DGPU();
		Line2DGPU revoked_line = deserilize_from_string<Line2DGPU>(content);
		newLine->Copy(&revoked_line);
		ent = newLine;
		break;
	}
	case EntityType::Circle:
	{
		Circle2DGPU* newCircle = new Circle2DGPU();
		Circle2DGPU revoked_circle = deserilize_from_string<Circle2DGPU>(content);
		newCircle->Copy(&revoked_circle);
		ent = newCircle;
		break;
	}
	case EntityType::Polyline:
	{
		Polyline2DGPU* newPoly = new Polyline2DGPU();
		Polyline2DGPU reoked_poly = deserilize_from_string<Polyline2DGPU>(content);
		newPoly->Copy(&reoked_poly);
		ent = newPoly;
		break;
	}
	case EntityType::Arc:
	{
		Arc2DGPU* newArc = new Arc2DGPU();
		Arc2DGPU reovked_arc = deserilize_from_string<Arc2DGPU>(content);
		newArc->Copy(&reovked_arc);
		ent = newArc;
		break;
	}
	case EntityType::Spline:
	{
		Arc2DGPU* newSpline = new Arc2DGPU();
		Arc2DGPU revoked_spline = deserilize_from_string<Arc2DGPU>(content);
		newSpline->Copy(&revoked_spline);
		ent = newSpline;
		break;
	}
	}
	return ent;
}
