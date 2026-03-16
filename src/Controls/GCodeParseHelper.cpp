#include "Controls/GCodeParseHelper.h"

bool parseGLine(const std::string& line, GCodeLine& out)
{
    // 清空
    out = {};

    // 匹配 Nxxx
    std::regex nRegex(R"(N(\d+))");
    std::smatch sm;
    if (std::regex_search(line, sm, nRegex)) {
        out.line = std::stoi(sm[1]);
    }

    // 匹配 G01/G02/G03/G05
    std::regex gRegex(R"(G(00|01|02|03|05))");
    if (std::regex_search(line, sm, gRegex)) {
        if (sm[0].str() == "G00")
        {
            out.type = GCodeLine::G00;
        }
        else if (sm[0].str() == "G01")
        {
            out.type = GCodeLine::G01;
        }
        else if (sm[0].str() == "G02")
        {
            out.type = GCodeLine::G02;
        }
        else if (sm[0].str() == "G03")
        {
            out.type = GCodeLine::G03;
        }
        else if (sm[0].str() == "G05")
        {
            out.type = GCodeLine::G05;
        }
    }
    else {
        return false; // 不是我们要的G代码
    }

    // 匹配 X Y I J
    std::regex xRegex(R"(X([+-]?\d+\.?\d*))");
    std::regex yRegex(R"(Y([+-]?\d+\.?\d*))");
    std::regex iRegex(R"(I([+-]?\d+\.?\d*))");
    std::regex jRegex(R"(J([+-]?\d+\.?\d*))");

    if (std::regex_search(line, sm, xRegex)) out.x = std::stod(sm[1]);
    if (std::regex_search(line, sm, yRegex)) out.y = std::stod(sm[1]);
    if (std::regex_search(line, sm, iRegex)) {
        if (out.type == GCodeLine::G02 || out.type)
        {
            out.i = std::stod(sm[1]);
        }
    }
    if (std::regex_search(line, sm, jRegex)) {
        if (out.type == GCodeLine::G03 || out.type)
        {
            out.j = std::stod(sm[1]);
        }
    }

    return true;
}

GCodeParseHelper::GCodeParseHelper(SketchGPU* sketch) : holdSketch(sketch)
{

}

GCodeParseHelper::~GCodeParseHelper()
{

}

void GCodeParseHelper::ParseFileToSketch(const std::string fileName)
{
    std::ifstream file(fileName,std::ios::in);
    if (!file.is_open())
    {
        g_file_logger->error("打开Nc文件{} 失败", fileName);
        return;
    }
    else
    {
        std::string line;
        int lineNum = 0;
        GCodeLine gLine;
        while (std::getline(file, line))
        {
            lineNum++;
            if (parseGLine(line, gLine))
            {
                glines.push_back(gLine);
            }
        }
    }

    glm::vec3 lastPoint = glm::vec3(0.0,0.0,0.0);

    for (GCodeLine& rec : glines)
    {
        switch (rec.type)
        {
            case GCodeLine::G00:
            {
                break;
            }
            case GCodeLine::G01:
            {
                Line2DGPU* line = new Line2DGPU(lastPoint, glm::vec3(rec.x, rec.y, 0.0));
                if (holdSketch)
                {
                    holdSketch->AddEntity(line);
                }
                break;
            }
            case GCodeLine::G02:
            {
                glm::vec3 start = lastPoint;
                glm::vec3 center = start + glm::vec3(rec.i, rec.j, 0.0f);
                glm::vec3 end = start + glm::vec3(rec.x, rec.y, 0.0f);
                float radius = glm::distance(start,center);
                float startAngle = MathUtils::GetCounterClockwiseAngle(center, glm::vec3(1, 0, 0), start);
                float endAngle = MathUtils::GetCounterClockwiseAngle(center, glm::vec3(1, 0, 0), end);
                Arc2DGPU* arc = new Arc2DGPU(center,radius,startAngle,endAngle);
                if (holdSketch)
                {
                    holdSketch->AddEntity(arc);
                }

                break;
            }
        }
        lastPoint = glm::vec3(rec.x, rec.y, 0.0);
    }
    
    if (holdSketch)
    {
        holdSketch->UpdateSketch();
    }
}
