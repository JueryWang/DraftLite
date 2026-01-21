#include "Algorithm/RoughingAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include <map>
#include <QMessageBox>
CNCSYS::EntityVGPU* RoughingAlgo::s_roughingPoly = nullptr;

std::string RoughingAlgo::GetRoughingPath(EntRingConnection* shape, const AABB& workblank, RoughingParamSettings setting)
{
    if (RoughingAlgo::s_roughingPoly != nullptr)
    {
        g_canvasInstance->GetSketchShared()->EraseEntity(s_roughingPoly);
        delete RoughingAlgo::s_roughingPoly;
    }

    std::string gcode;

    int PRECISION = (int)(1.0 / setting.tolerance);
    Polyline2DGPU* ultimatePoly = new Polyline2DGPU();
    //单工序
    glm::vec3 toolPos;
    glm::vec3 workpieceCentroid = shape->centroid;
    //先求余量的偏置
    Polyline2DGPU* originShape = static_cast<Polyline2DGPU*>(shape->ToPolyline());
    Polyline2DGPU* offsetAllowance = static_cast<Polyline2DGPU*>(originShape->Offset(setting.allowance, 1000));
    Clipper2Lib::Path64 allowancePath;
    for (const BoostPoint& p : offsetAllowance->boostPath)
    {
        allowancePath.emplace_back(bg::get<0>(p) * PRECISION, bg::get<1>(p) * PRECISION);
    }
    
    //偏置渐开线
    std::vector<Clipper2Lib::Path64> involute_sequence;
    std::map<Clipper2Lib::Path64*, Clipper2Lib::Paths64> involute_clippMap;

    Clipper2Lib::Path64 workBox;
    workBox.emplace_back((int)workblank.XRange() * PRECISION, (int)workblank.YRange() * PRECISION);
    workBox.emplace_back(0, (int)workblank.YRange() * PRECISION);
    workBox.emplace_back(0,0);
    workBox.emplace_back((int)workblank.XRange() * PRECISION, 0);
    
    //当前刀具所在位置
    toolPos = workblank.getMax() + glm::vec3(setting.stepover,setting.stepover,0.0f);

    std::vector<Path64> clipSections;
    
    std::vector<Polyline2DGPU*> layerPolys;

    int step = 0;
    do
    {
        ClipperOffset offset;
        Paths64  subject;
        subject.push_back(allowancePath);
        offset.AddPaths(subject, JoinType::Round, EndType::Polygon);

        Paths64 solution;
        double delta = step * setting.stepover;
        offset.Execute(delta * PRECISION,solution);
        involute_sequence.push_back(solution[0]);

        step++;
        clipSections = GetIntersections(involute_sequence.back(), workBox);

        std::vector<glm::vec3> clippingNodes;
        involute_clippMap[&involute_sequence.back()] = clipSections;
        for (const Path64 clipping : clipSections)
        {
            for (const Point64& pt : clipping)
            {
                clippingNodes.push_back({ (float)pt.x / PRECISION,(float)pt.y / PRECISION,0.0f });
            }
        }
        if (clippingNodes.size() > 0)
        {
            Polyline2DGPU* sectionLine = new Polyline2DGPU();
            sectionLine->SetParameter(clippingNodes,false);
            sectionLine->attribColor = g_yellowColor;
            sectionLine->ResetColor();
            layerPolys.push_back(sectionLine);
        }
        
    } while (clipSections.size() > 0);
    
    Polyline2DGPU* temp = *layerPolys.begin();
    delete temp;
    layerPolys.erase(layerPolys.begin());

    //前后两段线分别向外外扩展
    for (Polyline2DGPU* layer : layerPolys)
    {
        layer->ExtendStart(setting.toolRadius);
        layer->ExtendEnd(setting.toolRadius);
        layer->UpdatePaintData();
    }

    std::reverse(layerPolys.begin(),layerPolys.end());

    std::vector<glm::vec3> ultimatePath;

    char buffer[256];
    for (Polyline2DGPU* layer : layerPolys)
    {
        if (setting.direction == MillingDirection::CW)
        {
            layer->Reverse();
        }
        auto nodes = layer->GetTransformedNodes();
        
        {
            std::sprintf(buffer,"N%03d G00 X%f Y%f\n",g_MScontext.ncstep,nodes[0].x - g_MScontext.wcsAnchor,nodes[0].y - g_MScontext.wcsAnchor);
            gcode += buffer;
            g_MScontext.ncstep++;
            GCodeRecord rec(std::string(buffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
            GCodeController::GetController()->AddRecord(rec);
        }

        if (setting.direction == MillingDirection::Any)
        {
            glm::vec3 start = layer->GetStart();
            glm::vec3 end = layer->GetEnd();
            if (glm::distance(start, toolPos) > glm::distance(end, toolPos))
            {
                layer->Reverse();
                layer->UpdatePaintData();
                std::reverse(nodes.begin(),nodes.end());
                std::swap(start, end);
            }
            toolPos = end;
        }
        else
        {
            if (ultimatePath.size() > 0)
            {
                Path64 marchingline;
                glm::vec3 start = ultimatePath.back();
                glm::vec3 end = *nodes.begin();

                //产生碰撞,插补点
                marchingline.emplace_back(start.x * PRECISION ,start.y * PRECISION);
                marchingline.emplace_back(end.x * PRECISION, end.y * PRECISION);
                if (GetIntersections(marchingline, involute_sequence[1]).size() > 0)
                {
                    glm::vec3 interp = glm::vec3(max(start.x, end.x), max(start.y, end.y), 0.0f);
                    ultimatePath.push_back(interp);

                    {
                        std::sprintf(buffer, "N%03d G00 X%f Y%f\n", g_MScontext.ncstep, max(start.x, end.x), max(start.y, end.y));
                        gcode += buffer;
                        g_MScontext.ncstep++;
                        GCodeRecord rec(std::string(buffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
                        GCodeController::GetController()->AddRecord(rec);
                    }
                }
            }
        }
        gcode += layer->GenNcSection(&g_MScontext,true);

        ultimatePath.insert(ultimatePath.end(),nodes.begin(),nodes.end());
        delete layer;
    }

    s_roughingPoly = ultimatePoly;
    if (ultimatePath.size() > 0)
    {
        ultimatePoly->SetParameter(ultimatePath,false);
    }
    else
    {
        clipSections = GetIntersections(allowancePath, workBox);
        std::vector<glm::vec3> nodes;
        for (const Path64& clipping : clipSections)
        {
            for (const Point64& pt : clipping)
            {
                nodes.push_back({ (float)pt.x / PRECISION,(float)pt.y / PRECISION,0.0f });
            }
        }
        ultimatePoly->SetParameter(nodes, false);
        gcode += ultimatePoly->ToNcInstruction(&g_MScontext, true);
    }
    ultimatePoly->attribColor = g_yellowColor;
    ultimatePoly->ResetColor();
    g_canvasInstance->GetSketchShared()->AddEntity(s_roughingPoly);
    ultimatePoly->SelfAmendArcSection();

    delete originShape;
    delete offsetAllowance;

    return gcode;
}

std::vector<Path64> RoughingAlgo::GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB)
{
    Paths64 subject;
    subject.push_back(pathA);

    Paths64 clip;
    clip.push_back(pathB);

    Clipper64 clipper;

    clipper.AddOpenSubject(subject);
    clipper.AddClip(clip);

    Paths64 solution_closed;
    Paths64 solution_open;
    clipper.Execute(ClipType::Intersection, FillRule::NonZero, solution_closed, solution_open);
    return solution_open;
}
