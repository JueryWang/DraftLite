#include "Algorithm/RoughingAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include <map>
#define PRECISION 1000

Polyline2DGPU* RoughingAlgo::GetRoughingPath(EntRingConnection* shape, const AABB& workblank, double toolRadius, double stepover,double allowance)
{
    glm::vec3 toolPos;

    stepover = 10;
    //先求余量的偏置
    Polyline2DGPU* originShape = static_cast<Polyline2DGPU*>(shape->ToPolyline());
    Polyline2DGPU* offsetAllowance = static_cast<Polyline2DGPU*>(originShape->Offset(allowance, 1000));
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
    toolPos = workblank.getMax() + glm::vec3(toolRadius,toolRadius,0.0f);
    Point2DGPU* toolInitPos = new Point2DGPU(toolPos);
    g_canvasInstance->GetSketchShared()->AddEntity(toolInitPos);

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
        double delta = step * stepover;
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
            //g_canvasInstance->GetSketchShared()->AddEntity(sectionLine);
            layerPolys.push_back(sectionLine);
        }
        
    } while (clipSections.size() > 0);
    
    Polyline2DGPU* temp = *layerPolys.begin();
    delete temp;
    layerPolys.erase(layerPolys.begin());

    //前后两段线分别向外外扩展
    for (Polyline2DGPU* layer : layerPolys)
    {
        layer->ExtendStart(toolRadius);
        layer->ExtendEnd(toolRadius);
        layer->UpdatePaintData();
    }

    std::reverse(layerPolys.begin(),layerPolys.end());

    std::vector<glm::vec3> ultimatePath;

    for (Polyline2DGPU* layer : layerPolys)
    {
        glm::vec3 start = layer->GetStart();
        glm::vec3 end = layer->GetEnd();
        if (glm::distance(start, toolPos) > glm::distance(end, toolPos))
        {
            layer->Reverse();
            layer->UpdatePaintData();
            std::swap(start, end);
        }
        auto transformedNodes = layer->GetTransformedNodes();
        ultimatePath.insert(ultimatePath.end(), transformedNodes.begin(), transformedNodes.end());
        
        toolPos = end;
        delete layer;
    }

    Polyline2DGPU* ultimatePoly = new Polyline2DGPU();
    ultimatePoly->SetParameter(ultimatePath,false);
    ultimatePoly->attribColor = g_yellowColor;
    ultimatePoly->ResetColor();
    g_canvasInstance->GetSketchShared()->AddEntity(ultimatePoly);
    
    delete originShape;
    delete offsetAllowance;
    return ultimatePoly;
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
