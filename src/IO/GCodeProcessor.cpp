#include "IO/GCodeProcessor.h"

GCodeProcessor::GCodeProcessor(SketchGPU* sketch) : mSketch(sketch)
{

}

GCodeProcessor::~GCodeProcessor()
{

}

bool GCodeProcessor::LoadFromFile(const std::string& filePath)
{
	return false;
}

bool GCodeProcessor::SaveToFile(const std::string& filePath)
{
	std::string content = mSketch->ToNcProgram();
	std::ofstream outFile(filePath);
	std::cout << "filePath = " << filePath << std::endl;
	if (!outFile.is_open()) {
		std::cerr << "无法打开文件进行写入！" << std::endl;
		return 1;
	}
	outFile << content << std::endl;
	outFile.close();

	return true;
}
