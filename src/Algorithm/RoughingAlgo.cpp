#include "Algorithm/RoughingAlgo.h"
#include "Algorithm/ClusterAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include "Common/OpenGLContext.h"
#include <list>
#include <map>
#include <chrono>

std::vector<CNCSYS::EntityVGPU*> RoughingAlgo::s_cache;
int RoughingAlgo::percision;

std::string RoughingAlgo::GetRoughingPath(EntRingConnection* shape, const AABB& workblank, RoughingParamSettings setting)
{
    if (RoughingAlgo::s_cache.size())
    {
        for (EntityVGPU* path : RoughingAlgo::s_cache)
        {
            g_canvasInstance->GetSketchShared()->EraseEntity(path);
            delete path;
        }
        RoughingAlgo::s_cache.clear();
    }

    std::string gcode;

    RoughingAlgo::percision = 1.0/setting.tolerance;
    glm::vec3 workpieceCentroid = shape->centroid;
    //����������ƫ��
    Polyline2DGPU* originShape = static_cast<Polyline2DGPU*>(shape->ToPolyline());
    Polyline2DGPU* offsetAllowance = static_cast<Polyline2DGPU*>(originShape->Offset(setting.allowance, 1000));
    //余量轮廓
    Clipper2Lib::Path64 allowancePath;
    for (const BoostPoint& p : offsetAllowance->boostPath)
    {
        allowancePath.emplace_back(bg::get<0>(p) * RoughingAlgo::percision, bg::get<1>(p) * RoughingAlgo::percision);
    }

    //ƫ�ý�����
    std::vector<Clipper2Lib::Path64> involute_sequence;

    Clipper2Lib::Path64 workBox;
    workBox.emplace_back((int)workblank.XRange() * RoughingAlgo::percision, (int)workblank.YRange() * RoughingAlgo::percision);
    workBox.emplace_back(0, (int)workblank.YRange() * RoughingAlgo::percision);
    workBox.emplace_back(0, 0);
    workBox.emplace_back((int)workblank.XRange() * RoughingAlgo::percision, 0);

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
        offset.Execute(delta * RoughingAlgo::percision, solution);
        involute_sequence.push_back(solution[0]);

        step++;
        clipSections = GetIntersections(involute_sequence.back(), workBox);

        std::vector<glm::vec3> clippingNodes;

        for (const Path64 clipping : clipSections)
        {
            for (const Point64& pt : clipping)
            {
                clippingNodes.push_back({ (float)pt.x / RoughingAlgo::percision,(float)pt.y / RoughingAlgo::percision,0.0f });
            }
            if (clippingNodes.size() > 0)
            {
                Polyline2DGPU* sectionLine = new Polyline2DGPU();
                sectionLine->SetParameter(clippingNodes, false);
                sectionLine->attribColor = g_yellowColor;
                sectionLine->ResetColor();
                layerPolys.push_back(sectionLine);
                //最内层0
                clusterNodes.push_back(PointClusterNode(sectionLine->centroid, involute_sequence.size()-1, sectionLine));
                if (firstOffset)
                {
                    clusterInitCenter.push_back(PointClusterNode(sectionLine->centroid, involute_sequence.size()-1, sectionLine));
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
    
    //翻转,最外层层级为0
    for (PointClusterNode& pointCluster : clusterNodes)
    {
        pointCluster.clippingLayer = (involute_sequence.size()-1) - pointCluster.clippingLayer;
    }
    std::reverse(involute_sequence.begin(),involute_sequence.end());
    //先聚类,区分出各截断区域
    std::map<int, std::vector<PointClusterNode>> pointSet = PointRegionCluster::kmeans(clusterNodes, clusterInitCenter, clusterInitCenter.size(), 10);
    //建图
    VisibilityGraph graph;
    //初始障碍物,单个区域切割完成后更新
    Path64 Obstacle;
    //添加障碍轮廓
    graph.addObstacle(involute_sequence.back());
    graph.SetPrecision(RoughingAlgo::percision);
    for (const Point64& pt : involute_sequence[0])
    {
        graph.addExtraPoint(pt);
    }

    bool toolInited = false;
    glm::vec3 lastEnd;
    char buffer[256];
    for (auto& pair : pointSet)
    {
        //由内往外翻转为由外向内
        std::reverse(pair.second.begin(), pair.second.end());

        std::vector<glm::vec3> sectionPath;
        Polyline2DGPU* sectionPoly = new Polyline2DGPU();
        glm::vec4 randomColor = GetRandomColor();

        for (int i = 0; i < pair.second.size(); i++)
        {
            PointClusterNode pNode = pair.second[i];
            EntityVGPU* layer = pNode.entityParent;
            
            glm::vec3 start = layer->GetStart();
            glm::vec3 end = layer->GetEnd();

            if (setting.direction == MillingDirection::CW)
            {
                layer->Reverse();
                std::swap(start, end);
            }
            auto nodes = layer->GetTransformedNodes();

            if (setting.direction == MillingDirection::Any)
            {
                if (glm::distance(start, lastEnd) > glm::distance(end, lastEnd))
                {
                    layer->Reverse();
                    std::swap(start, end);
                    layer->UpdatePaintData();
                    std::reverse(nodes.begin(), nodes.end());
                }
            }
            //区域跳转逻辑
            if (toolInited && i == 0)
            {
                Path64 marchingline;
                glm::vec3 lineS = lastEnd;
                glm::vec3 lineEd = start;
                marchingline.emplace_back(lineS.x * RoughingAlgo::percision, lineS.y * RoughingAlgo::percision);
                marchingline.emplace_back(lineEd.x * RoughingAlgo::percision, lineEd.y * RoughingAlgo::percision);
                if (GetIntersections(marchingline, involute_sequence[0]).size() > 0)
                {
                    InterpToEscape(lineS, lineEd, graph, gcode);
                }
                else
                {
                    Line2DGPU* line = new Line2DGPU();
                    line->SetParameter(lineS, lineEd);
                    line->attribColor = GetRandomColor();
                    line->ResetColor();
                    std::sprintf(buffer,"N%3d G00 X%f Y%f \n",g_MScontext.ncstep,lineEd.x - g_MScontext.wcsAnchor.x,lineEd.y - g_MScontext.wcsAnchor.y);
                    gcode += buffer;
                    g_MScontext.ncstep++;
                    g_canvasInstance->GetSketchShared()->AddEntity(line);
                    s_cache.push_back(line);
                }
            }
            else if (toolInited)
            {
                int collisionLayer = (pair.second[i].clippingLayer+1) > (involute_sequence.size() - 1) ? (involute_sequence.size() - 1) : (pair.second[i].clippingLayer + 1);
                Path64 marchingline;
                glm::vec3 lineS = lastEnd;
                glm::vec3 lineEd = start;
                marchingline.emplace_back(lineS.x* RoughingAlgo::percision, lineS.y* RoughingAlgo::percision);
                marchingline.emplace_back(lineEd.x* RoughingAlgo::percision, lineEd.y* RoughingAlgo::percision);
                
                if (GetIntersections(marchingline, involute_sequence[collisionLayer]).size() > 0)
                {

                    for (Point64& pt : involute_sequence[0])
                    {
                        //匹配最外层最近点

                    }
                    Polyline2DGPU* newSection = new Polyline2DGPU();
                    newSection->SetParameter(sectionPath, false);
                    newSection->attribColor = g_yellowColor;
                    newSection->ResetColor();
                    g_canvasInstance->GetSketchShared()->AddEntity(newSection);
                    s_cache.push_back(newSection);
                    sectionPath.clear();

                    graph.ClearGraph();
                    if (collisionLayer == (involute_sequence.size() - 1))
                    {
                        graph.addObstacle(allowancePath);
                    }
                    else
                    {
                        graph.addObstacle(involute_sequence[collisionLayer]);
                    }
                    for (const Point64& pt : involute_sequence[0])
                    {
                        graph.addExtraPoint(pt);
                    }
                    InterpToEscape(lineS, lineEd, graph, gcode);
                }

            }

            lastEnd = nodes.back();
            gcode += layer->GenNcSection(&g_MScontext, false);
            sectionPath.insert(sectionPath.end(), nodes.begin(), nodes.end());
            //Paths64 line;
            //line.push_back(MakePath({(int)(nodes[0].x * PRECISION),int(nodes[0].y * PRECISION),int(nodes.back().x * PRECISION),int(nodes.back().y * PRECISION)}));
            //Paths64 removed = InflatePaths(line, setting.toolRadius, JoinType::Round, EndType::Round);
            //auto dif = Difference({ workBox }, removed, FillRule::EvenOdd);
            //workBox = dif[0];
            delete layer;
        }
        toolInited = true;

        //Polyline2DGPU* remainedPoly = new Polyline2DGPU();
        //std::vector<glm::vec3> remainPolyNodes;
        //for (const Point64& pt : workBox)
        //{
        //    remainPolyNodes.push_back({ (float)pt.x / PRECISION,(float)pt.y / PRECISION,0.0f });
        //}
        //remainedPoly->attribColor = GetRandomColor();
        //remainedPoly->ResetColor();
        //g_canvasInstance->GetSketchShared()->AddEntity(remainedPoly);


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
                    nodes.push_back({ (float)pt.x / RoughingAlgo::percision,(float)pt.y / RoughingAlgo::percision,0.0f });
                }
            }
            sectionPoly->SetParameter(nodes, false);
            gcode += sectionPoly->ToNcInstruction(&g_MScontext, false);
        }
        sectionPoly->attribColor = g_yellowColor;
        sectionPoly->ResetColor();
        g_canvasInstance->GetSketchShared()->AddEntity(sectionPoly);
        s_cache.push_back(sectionPoly);
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

void RoughingAlgo::InterpToEscape(const glm::vec3 start, const glm::vec3 end, VisibilityGraph& vGraph, std::string& gcode)
{
    char buffer[256];
    vGraph.buildGraph(Point64(start.x * RoughingAlgo::percision, start.y * RoughingAlgo::percision), Point64(end.x * RoughingAlgo::percision, end.y * RoughingAlgo::percision));
    std::vector<glm::vec3> path = vGraph.solve();
    if (path.size() > 2)
    {
        Polyline2DGPU* interpPoly = new Polyline2DGPU();
        path.insert(path.begin(), start);
        path.push_back(end);
        interpPoly->SetParameter(path, false);
        interpPoly->attribColor = GetRandomColor();
        interpPoly->ResetColor();
        for (int i = 1; i < path.size(); i++)
        {
            std::sprintf(buffer, "N%3d G00 X%f Y%f \n", g_MScontext.ncstep, path[i].x - g_MScontext.wcsAnchor.x, path[i].y - g_MScontext.wcsAnchor.y);
            gcode += buffer;
            g_MScontext.ncstep++;
        }
        g_canvasInstance->GetSketchShared()->AddEntity(interpPoly);
        s_cache.push_back(interpPoly);
    }
    else
    {
        Line2DGPU* line = new Line2DGPU();
        line->SetParameter(start,end);
        line->attribColor = g_redColor;
        line->ResetColor();
        g_canvasInstance->GetSketchShared()->AddEntity(line);
        s_cache.push_back(line);
    }
}
