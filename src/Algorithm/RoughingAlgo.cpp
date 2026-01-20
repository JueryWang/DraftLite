#include "Algorithm/RoughingAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include <map>
#define PRECISION 1000

Polyline2DGPU* RoughingAlgo::GetRoughingPath(EntRingConnection* shape, const AABB& workblank, RoughingParamSettings setting)
{
    Polyline2DGPU* ultimatePoly = new Polyline2DGPU();
    //判断是否需要多段工序
    if (shape->bbox.getMax().x < (workblank.getMax().x + setting.toolRadius) &&
        shape->bbox.getMin().x >(shape->bbox.getMin().x - setting.toolRadius) &&
        shape->bbox.getMax().y < (workblank.getMax().y + setting.toolRadius) &&
        shape->bbox.getMin().y >(shape->bbox.getMin().y - setting.toolRadius)
        )
    {
        //单工序
        glm::vec3 toolPos;
        glm::vec3 workpieceCentroid = shape->centroid;

        setting.stepover = 10;
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
        toolPos = workblank.getMax() + glm::vec3(setting.toolRadius,setting.toolRadius,0.0f);

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

        for (Polyline2DGPU* layer : layerPolys)
        {
            auto nodes = layer->GetTransformedNodes();
            glm::vec3 pathDir = nodes[1] - nodes[0];
            glm::vec3 peiceDir = nodes[1] - workpieceCentroid;
            //逆时针
            if (glm::cross(pathDir, peiceDir).z > 0)
            {
                if (setting.direction == MillingDirection::CCW)
                {
                    std::reverse(nodes.begin(),nodes.end());
                }
            }
            //顺时针
            if (glm::cross(pathDir, peiceDir).z < 0)
            {
                if (setting.direction == MillingDirection::CW)
                {
                    std::reverse(nodes.begin(), nodes.end());
                }
            }
        
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
                }
            }

            ultimatePath.insert(ultimatePath.end(),nodes.begin(),nodes.end());
            delete layer;
        }

        ultimatePoly->SetParameter(ultimatePath,false);
        ultimatePoly->attribColor = g_yellowColor;
        ultimatePoly->ResetColor();
        g_canvasInstance->GetSketchShared()->AddEntity(ultimatePoly);
    
        delete originShape;
        delete offsetAllowance;
    }

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
