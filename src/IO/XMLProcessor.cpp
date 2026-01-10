#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include "IO/XMLProcessor.h"
#include "IO/ObjectSerializer.h"
#include "UI/TaskFlowGuide.h"
#include "UI/TaskListWindow.h"
#include "UI/MainLayer.h"
#include "Common/ProgressInfo.h"
#include "Common/CraftParamConfig.h"
#include "Controls/GlobalPLCVars.h"
#include "NetWork/MessageValidtor.h"
#include "UI/TaskListWindow.h"
#include "IO/DxfProcessor.h"
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <IO/Utils.h>
#include <QUUID>
#include <QProcess>
#include <QApplication>

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
	DecompressProject(FileName, tempDir);
	QString pureName = SplitFileFromPath(FileName);
	QString conpressedPath = tempDir + "/" + pureName;
	TaskListWindow::GetInstance()->ClearItems();

	QFile file(conpressedPath + "/" + pureName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return;
	}

	QXmlStreamReader reader(&file);
	while (!reader.atEnd())
	{
		QXmlStreamReader::TokenType token = reader.readNext();
		if (token == QXmlStreamReader::StartElement && reader.name() == QString("Scene"))
		{
			ParseScene(reader, conpressedPath);
		}

		// 跳过空白节点
		if (token == QXmlStreamReader::Characters && reader.isWhitespace())
		{
			continue;
		}
	}
}


void XMLProcessor::SaveScene(QXmlStreamWriter& writer, CNCSYS::SketchGPU* sketch, const QString& TempDir)
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

	auto groups = sketch->GetEntityGroups();
	for (EntGroup* group : groups)
	{
		for (EntRingConnection* ring : group->rings)
		{
			for (EntityVGPU* ent : ring->conponents)
			{
				
			}
		}
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

	writer.writeEndElement();
}

void XMLProcessor::ParseScene(QXmlStreamReader& reader, const QString& projPath)
{
	if (reader.name() != QString("Scene") || reader.tokenType() != QXmlStreamReader::StartElement)
	{
		reader.skipCurrentElement();
		return;
	}

	QXmlStreamAttributes attrs = reader.attributes();


	QString layerId;
	if (attrs.hasAttribute("id"))
	{
		layerId = attrs.value("id").toString();

	}
	CNCSYS::SketchGPU* createdSketch = nullptr;
	if (attrs.hasAttribute("Source"))
	{
		QString fileSource = projPath + "/" + attrs.value("Source").toString();
		if (layerId != "Default Layer")
		{
			if (!fileSource.isEmpty())
			{
				ToDoListItem* itemNew = new ToDoListItem();
				ToDoListItemWidget* itemWidget = new ToDoListItemWidget(fileSource, TaskListWindow::GetInstance());
				createdSketch = itemWidget->sketch.get();
				TaskListWindow::GetInstance()->AddTaskItem(itemNew, itemWidget);
				itemWidget->fileSource = attrs.value("Source").toString();
			}

		}
		if (layerId == "Default Layer")
		{
			if (!fileSource.isEmpty())
			{
				DXFProcessor processor(g_canvasInstance->GetSketchShared());
				processor.read(fileSource.toStdString());
				g_canvasInstance->UpdateOCS();
				g_canvasInstance->GetSketchShared()->UpdateGCode(true);
			}
		}
	}

	CraftConfigItems configItems;
	while (!reader.atEnd())
	{
		QXmlStreamReader::TokenType token = reader.readNext();
		if (token == QXmlStreamReader::EndElement && reader.name() == QString("Scene"))
		{
			break;
		}

		if (token == QXmlStreamReader::StartElement && reader.name() == QString("CraftParams")) {
			CraftParamConfig config;
			AtomicVarType varType;
			while (!(reader.readNextStartElement() == false && reader.name() == QString("CraftParams"))) {
				// 如果读到了子元素的开始标签
				if (reader.isStartElement()) {
					QString name = reader.name().toString();
					QString text = reader.readElementText();

					if (name == "alias")
					{
						config.alias = text.toStdString();
					}
					else if (name == "type")
					{
						varType = static_cast<AtomicVarType>(text.toInt());
					}
					else if (name == "address")
					{
						std::string address = text.toStdString();
						for (auto& pair : g_PLCVariables)
						{
							PLCParam_ProtocalOpc* opcInfo = static_cast<PLCParam_ProtocalOpc*>(pair.second);
							if (opcInfo != NULL)
							{
								if (strcmp(opcInfo->identifier, address.c_str()) == 0)
								{
									config.plcInfo = opcInfo;
								}
							}
						}
					}
					else if (name == "value")
					{
						switch (varType)
						{
						case AtomicVarType::BOOL:
						{
							if (text == "True")
							{
								config.preSetVal = true;
							}
							else if (text == "False")
							{
								config.preSetVal = false;
							}
							break;
						}
						case AtomicVarType::WORD:
						{
							uint16_t value;
							if (stringToUint16(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::DWORD:
						{
							uint32_t value;
							if (stringToUint32(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::LWORD:
						{
							uint64_t value;
							if (stringToUint64(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::INT:
						{
							int16_t value;
							if (stringToInt16(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::DINT:
						{
							int32_t value;
							if (stringToInt32(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::LINT:
						{
							int64_t value;
							if (stringToInt64(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::REAL:
						{
							double value;
							if (stringToDouble(text, value))
							{
								config.preSetVal = value;
							}
							break;
						}
						case AtomicVarType::STRING:
						{
							config.preSetVal = text.toStdString();
							break;
						}
						}
					}
				}

				if (reader.atEnd()) break;
			}
			configItems.push_back(config);
		}
	}
	if (createdSketch)
	{
		createdSketch->attachedConfig = configItems;
	}
}
