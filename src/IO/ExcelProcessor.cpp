#include "IO/ExcelProcessor.h"
#include "Controls/GlobalPLCVars.h"
#include "Common/Program.h"
#include "QXlsxQt6/xlsxworksheet.h"
#include "QXlsxQt6/xlsxworkbook.h"
#include "Controls/ScadaScheduler.h"
#include <UI/Components/HmiTemplateTableMonitor.h>
#include <UI/Components/HmiTemplateMsgBox.h>
#include <QFileDialog>

ExcelProcessor::ExcelProcessor()
{

}

ExcelProcessor::~ExcelProcessor()
{

}

void ExcelProcessor::WritePLC_OPCUAVariantMap()
{
	QString configFile = QFileDialog::getSaveFileName(nullptr,
		"选择保存的变量配置文件",
		QDir::homePath(),
		"变量表文件(*.wgp);;所有文件(*)"
	);


	QXlsx::Document document;

	// 设置标题样式
	QXlsx::Format titleFormat;
	titleFormat.setFontBold(true);          // 粗体
	titleFormat.setFontSize(14);           // 字体大小
	titleFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter); // 水平居中
	titleFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);   // 垂直居中
	titleFormat.setFillPattern(QXlsx::Format::PatternSolid);         // 填充样式
	titleFormat.setPatternBackgroundColor(QColor(200, 220, 255));                 // 填充颜色

	// 设置表头样式
	QXlsx::Format headerFormat;
	headerFormat.setFontBold(true);
	headerFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
	headerFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
	headerFormat.setBorderStyle(QXlsx::Format::BorderThin);           // 边框
	headerFormat.setPatternBackgroundColor(QColor(220, 230, 255));

	// 设置内容样式
	QXlsx::Format contentFormat;
	contentFormat.setBorderStyle(QXlsx::Format::BorderThin);
	contentFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);

	document.write(1, 1, QVariant(QString("变量名")));
	document.write(1, 2, QVariant(QString("类型")));
	document.write(1, 3, QVariant(QString("地址")));

	int row = 2;
	for (auto& pair : g_PLCVariables)
	{
		PLCParam_ProtocalOpc* info = (PLCParam_ProtocalOpc*)pair.second;
		if (info)
		{
			document.write(row, 1, QVariant(QString::fromStdString(pair.first)), contentFormat);
			switch (info->dataType)
			{
			case AtomicVarType::BOOL:
			{
				document.write(row, 2, QVariant("BOOL"), contentFormat);
				break;
			}
			case AtomicVarType::WORD:
			{
				document.write(row, 2, QVariant("WORD"), contentFormat);
				break;
			}
			case AtomicVarType::DWORD:
			{
				document.write(row, 2, QVariant("DWORD"), contentFormat);
				break;
			}
			case AtomicVarType::LWORD:
			{
				document.write(row, 2, QVariant("LWORD"), contentFormat);
				break;
			}
			case AtomicVarType::INT:
			{
				document.write(row, 2, QVariant("INT"), contentFormat);
				break;
			}
			case AtomicVarType::DINT:
			{
				document.write(row, 2, QVariant("DINT"), contentFormat);
				break;
			}
			case AtomicVarType::LINT:
			{
				document.write(row, 2, QVariant("LINT"), contentFormat);
				break;
			}
			case AtomicVarType::REAL:
			{
				document.write(row, 2, QVariant("REAL"), contentFormat);
				break;
			}
			case AtomicVarType::LREAL:
			{
				document.write(row, 2, QVariant("LREAL"), contentFormat);
				break;
			}
			case AtomicVarType::STRING:
			{
				document.write(row, 2, QVariant("STRING"), contentFormat);
				break;
			}
			}
			document.write(row, 3, QVariant(QString("%1:%2").arg(info->ns).arg(QString::fromLocal8Bit(info->identifier))), contentFormat);
			row++;
		}
	}
	bool saved = document.saveAs(configFile);
}

void ExcelProcessor::ReadPLCVariantMap()
{
	ClearPLCVariablesOpcUA();
	regTags.clear();
	QString configFile = g_plcVarCnfigExcelUrl;

	QXlsx::Document xlsx(configFile);
	if (!xlsx.isLoadPackage())
	{
		HmiTemplateMsgBox::warning(nullptr, "错误", QString("从地址%1加载变量表错误,请检查config.ini内的配置").arg(configFile), { "","","确定" }, { nullptr,nullptr,nullptr });
		qWarning() << "加载Excel文件失败:" << configFile;
		return;
	}

	int rowCount = xlsx.dimension().rowCount();
	int colCount = xlsx.dimension().columnCount();

	for (int row = 2; row <= rowCount; ++row)
	{
		PLCParam_ProtocalOpc* plcInfo = new PLCParam_ProtocalOpc();
		std::string varTag = xlsx.read(row, 1).toString().toStdString();
		std::string varType = xlsx.read(row, 2).toString().toStdString();
		QString varDescription = xlsx.read(row, 3).toString();
		int colconIndex = varDescription.indexOf(':');

		if (colconIndex != -1)
		{
			QString ns = varDescription.left(colconIndex).trimmed();
			QString identifier = varDescription.mid(colconIndex + 1).trimmed();

			if (varType == "BOOL")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::BOOL, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "WORD")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::WORD, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "DWORD")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::DWORD, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "LWORD")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::LWORD, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "INT")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::INT, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "DINT")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::DINT, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "LINT")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::LINT, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "REAL")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::REAL, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "LREAL")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::LREAL, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "STRING")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::STRING, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "STRUCT")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::STRUCT, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
			else if (varType == "ARRAY_BOOL")
			{
				PLCInitOpcInfo(plcInfo, AtomicVarType::ARRAY_BOOL, varTag, ns.toInt(), identifier.toLocal8Bit().data());
				regTags.push_back(varTag);
			}
		}
	}

	for (auto& pair : g_ConfigableKeys)
	{
		if (std::find(regTags.begin(), regTags.end(), pair.second) == regTags.end())
		{
			g_file_logger->warn("OPC节点匹配失败,未查询到节点 - Tag:{} Identifier:{}",pair.first,pair.second);
		}
	}

	for (auto& pair : g_ConfigableKeys)
	{
		ScadaScheduler::GetInstance()->RegisterReadBackVarKey(pair.second);
	}
}
