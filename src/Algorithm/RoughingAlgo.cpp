#include "Algorithm/RoughingAlgo.h"
#include "Algorithm/ClusterAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include "Common/OpenGLContext.h"

#include <list>
#include <map>

CNCSYS::EntityVGPU* RoughingAlgo::s_roughingPoly;

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
    //������
    glm::vec3 workpieceCentroid = shape->centroid;
    //����������ƫ��
    Polyline2DGPU* originShape = static_cast<Polyline2DGPU*>(shape->ToPolyline());
    Polyline2DGPU* offsetAllowance = static_cast<Polyline2DGPU*>(originShape->Offset(setting.allowance, 1000));
    Clipper2Lib::Path64 allowancePath;
    for (const BoostPoint& p : offsetAllowance->boostPath)
    {
        allowancePath.emplace_back(bg::get<0>(p) * PRECISION, bg::get<1>(p) * PRECISION);
    }

    //ƫ�ý�����
    std::vector<Clipper2Lib::Path64> involute_sequence;
    std::map<Clipper2Lib::Path64*, Clipper2Lib::Paths64> involute_clippMap;

    Clipper2Lib::Path64 workBox;
    workBox.emplace_back((int)workblank.XRange() * PRECISION, (int)workblank.YRange() * PRECISION);
    workBox.emplace_back(0, (int)workblank.YRange() * PRECISION);
    workBox.emplace_back(0, 0);
    workBox.emplace_back((int)workblank.XRange() * PRECISION, 0);

    //刀具位置
    std::vector<Path64> clipSections;
    std::vector<Polyline2DGPU*> layerPolys;
    std::vector<PointClusterNode> clusterNodes;
    std::vector<PointClusterNode> clusterInitCenter;
    
    bool firstOffset = true;
    int step = 1;
    do
    {
        ClipperOffset offset;
        Paths64  subject;
        subject.push_back(allowancePath);
        offset.AddPaths(subject, JoinType::Round, EndType::Polygon);

        Paths64 solution;
        double delta = step * setting.stepover;
        offset.Execute(delta * PRECISION, solution);
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
            if (clippingNodes.size() > 0)
            {
                Polyline2DGPU* sectionLine = new Polyline2DGPU();
                sectionLine->SetParameter(clippingNodes, false);
                sectionLine->attribColor = g_yellowColor;
                sectionLine->ResetColor();
                layerPolys.push_back(sectionLine);
                clusterNodes.push_back(PointClusterNode(sectionLine->centroid, sectionLine));
                if (firstOffset)
                {
                    clusterInitCenter.push_back(PointClusterNode(sectionLine->centroid, sectionLine));
                }
            }
            clippingNodes.clear();
        }
        firstOffset = false;

    } while (clipSections.size());

    //首尾延长刀具半径值
    for (Polyline2DGPU* layer : layerPolys)
    {
        layer->ExtendStart(setting.toolRadius);
        layer->ExtendEnd(setting.toolRadius);
        layer->UpdatePaintData();
    }
    std::reverse(layerPolys.begin(), layerPolys.end());

    //先聚类,区分出各截断区域
    std::map<int, std::vector<PointClusterNode>> pointSet = PointRegionCluster::kmeans(clusterNodes, clusterInitCenter, clusterInitCenter.size(), 10);
    char buffer[256];
    bool toolInited = false;

    for (auto& pair : pointSet)
    {
        std::vector<glm::vec3> sectionPath;
        Polyline2DGPU* sectionPoly = new Polyline2DGPU();
        glm::vec4 randomColor = GetRandomColor();

        for (PointClusterNode& pNode : pair.second)
        {
            pNode.entityParent->attribColor = randomColor;
            pNode.entityParent->ResetColor();

            EntityVGPU* layer = pNode.entityParent;
            if (setting.direction == MillingDirection::CW)
            {
                layer->Reverse();
            }
            auto nodes = layer->GetTransformedNodes();
            if (!toolInited)
            {
                g_MScontext.toolPos = nodes[0];
                toolInited = true;
            }

            {
                Path64 marchingline;
                glm::vec3 start = g_MScontext.toolPos;
                glm::vec3 end = nodes[0];
                marchingline.emplace_back(start.x* PRECISION, start.y* PRECISION);
                marchingline.emplace_back(end.x* PRECISION, end.y* PRECISION);
                if (GetIntersections(marchingline, involute_sequence[0]).size() > 0)
                {
                    InterpToEscape(start, end, sectionPath, PRECISION, involute_sequence[0], gcode);
                }
            }

            if (setting.direction == MillingDirection::Any)
            {
                glm::vec3 start = layer->GetStart();
                glm::vec3 end = layer->GetEnd();
                if (glm::distance(start, g_MScontext.toolPos) > glm::distance(end, g_MScontext.toolPos))
                {
                    layer->Reverse();
                    layer->UpdatePaintData();
                    std::reverse(nodes.begin(), nodes.end());
                    std::swap(start, end);
                }
            }
            g_MScontext.toolPos = nodes.back();
            gcode += layer->GenNcSection(&g_MScontext, true);
            sectionPath.insert(sectionPath.end(), nodes.begin(), nodes.end());
            delete layer;
        }

        if (sectionPath.size() > 0)
        {
            sectionPoly->SetParameter(sectionPath, false);
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
            sectionPoly->SetParameter(nodes, false);
            gcode += sectionPoly->ToNcInstruction(&g_MScontext, true);
        }
        sectionPoly->attribColor = g_yellowColor;
        sectionPoly->ResetColor();
        g_canvasInstance->GetSketchShared()->AddEntity(sectionPoly);
    }

    delete originShape;
    delete offsetAllowance;

    return gcode;
}

Paths64 RoughingAlgo::GetIntersections(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB)
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

void RoughingAlgo::InterpToEscape(const glm::vec3 start, const glm::vec3 end, std::vector<glm::vec3>& path,int PRECISION,const Path64 barrier, std::string& gcode)
{
    char buffer[256];
    Path64 marchingline;
    glm::vec3 interp = glm::vec3(max(start.x, end.x), max(start.y, end.y), 0.0f);
    bool interpAdded = false;

    if (PointInPolygon(Point64(interp.x * PRECISION, interp.y * PRECISION), barrier) == PointInPolygonResult::IsOutside)
    {
        std::sprintf(buffer, "N%03d G00 X%f Y%f\n", g_MScontext.ncstep, interp.x - g_MScontext.wcsAnchor.x, interp.y - g_MScontext.wcsAnchor.y);
        gcode += buffer;
        g_MScontext.ncstep++;
        GCodeRecord rec(std::string(buffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
        GCodeController::GetController()->AddRecord(rec);
        gcode += buffer;
        interpAdded = true;
    }
    else
    {
        interp = glm::vec3(min(start.x, end.x), min(start.y, end.y), 0.0f);
        if(PointInPolygon(Point64(interp.x * PRECISION, interp.y * PRECISION), barrier) == PointInPolygonResult::IsOutside)
        {
            std::sprintf(buffer, "N%03d G00 X%f Y%f\n", g_MScontext.ncstep, interp.x - g_MScontext.wcsAnchor.x, interp.y - g_MScontext.wcsAnchor.y);
            gcode += buffer;
            g_MScontext.ncstep++;
            GCodeRecord rec(std::string(buffer), nullptr, -1, glm::mat4(1.0f), g_MScontext.ncstep);
            GCodeController::GetController()->AddRecord(rec);
            gcode += buffer;
            interpAdded = true;
        }
    }
    if (!interpAdded)
    {
        std::cout << "Interp Not Added!" << std::endl;
    }
    path.push_back(interp);
}
