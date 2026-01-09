#pragma once
#include "QXlsxQt6/xlsxdocument.h"
#include "QXlsxQt6/xlsxworksheet.h"
#include "QXlsxQt6/xlsxcellrange.h"
#include <string>

class ExcelProcessor
{
public:
	ExcelProcessor();
	~ExcelProcessor();

	void WritePLC_OPCUAVariantMap();
	void ReadPLCVariantMap();
public:
	std::vector<std::string> regTags;
};