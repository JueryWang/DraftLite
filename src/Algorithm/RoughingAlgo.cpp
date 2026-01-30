#include "Algorithm/RoughingAlgo.h"
#include "Algorithm/ClusterAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Controls/GCodeController.h"
#include "Common/OpenGLContext.h"
#include <list>
#include <map>
#include <chrono>
#include <random>

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

    //保存所有层的偏置结果
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

        clipSections = GetIntersections(involute_sequence.back(), workBox);

        std::vector<glm::vec3> clippingNodes;

        step++;
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
                clusterNodes.push_back(PointClusterNode(sectionLine->centroid, involute_sequence.size(), sectionLine));
                if (firstOffset)
                {
                    clusterInitCenter.push_back(PointClusterNode(sectionLine->centroid, involute_sequence.size(), sectionLine));
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
    std::map<int, std::vector<PointClusterNode>> pointSet = PointRegionCluster::kmeans(clusterNodes, clusterInitCenter, clusterInitCenter.size(), 50);
    //for (auto& pair : pointSet)
    //{
    //    glm::vec4 randomColor = GetRandomColor();
    //    for (PointClusterNode& pNode : pair.second)
    //    {
    //        pNode.entityParent->attribColor = randomColor;
    //        pNode.entityParent->ResetColor();
    //        g_canvasInstance->GetSketchShared()->AddEntity(pNode.entityParent);
    //    }
    //}
    ////建图
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
                    //Line2DGPU* line = new Line2DGPU();
                    //line->SetParameter(lineS, lineEd);
                    //line->attribColor = GetRandomColor();
                    //line->ResetColor();
                    //std::sprintf(buffer,"N%3d G00 X%f Y%f \n",g_MScontext.ncstep,lineEd.x - g_MScontext.wcsAnchor.x,lineEd.y - g_MScontext.wcsAnchor.y);
                    //gcode += buffer;
                    //g_MScontext.ncstep++;
                    //g_canvasInstance->GetSketchShared()->AddEntity(line);
                    //s_cache.push_back(line);
                }
            }
            else if (toolInited)
            {
                int collisionLayer = pair.second[i].clippingLayer;
                Path64 marchingline;
                glm::vec3 lineS = lastEnd;
                glm::vec3 lineEd = start;
                marchingline.emplace_back(lineS.x* RoughingAlgo::percision, lineS.y* RoughingAlgo::percision);
                marchingline.emplace_back(lineEd.x* RoughingAlgo::percision, lineEd.y* RoughingAlgo::percision);
                
                if (Intersect(marchingline, involute_sequence[collisionLayer]))
                {
					Point64 nearestPt;
                    double nearestDist = DBL_MAX;
                    if (collisionLayer < 1)
                        break;
                    for (Point64& pt : involute_sequence[max(collisionLayer-1,0)])
                    {
                        //匹配最外层与起始点最匹配的点
                        double dist = GetDistance(marchingline[0], pt);
                        if(dist < nearestDist)
                        {
                            nearestDist = dist;
                            nearestPt = pt;
						}
                    }

					std::vector<glm::vec3> probePts;
                    std::vector<Point64> probePtsPoint64;
                    probePts.push_back(lineS);
                    probePts.push_back({ (float)nearestPt.x/ RoughingAlgo::percision,(float)nearestPt.y/ RoughingAlgo::percision,0.0f});
                    int depth = 0;
                    ProbePath(nearestPt,marchingline[1], involute_sequence[collisionLayer], setting.stepover * RoughingAlgo::percision, probePtsPoint64,depth);
                    //TrimPath(probePtsPoint64, involute_sequence[collisionLayer]);
                    for (Point64& pt : probePtsPoint64)
                    {
                        probePts.push_back({(float)pt.x/ RoughingAlgo::percision,(float)pt.y/ RoughingAlgo::percision ,0.0f});
                    }
                    probePts.push_back(lineEd);
                    Polyline2DGPU* probePoly = new Polyline2DGPU();
                    probePoly->SetParameter(probePts, false);
                    probePoly->attribColor = GetRandomColor();
                    probePoly->ResetColor();
                    g_canvasInstance->GetSketchShared()->AddEntity(probePoly);
                    //{
                    //    Spline2DGPU* probeSpline = new Spline2DGPU();
                    //    std::vector<float> knots = MathUtils::GenerateClampedKnots(probePts.size(),3);
                    //    probeSpline->SetParameter(probePts,knots,true);
                    //    probeSpline->attribColor = GetRandomColor();
                    //    probeSpline->ResetColor();
                    //    g_canvasInstance->GetSketchShared()->AddEntity(probeSpline);
                    //}

                    Polyline2DGPU* newSection = new Polyline2DGPU();
                    newSection->SetParameter(sectionPath, false);
                    newSection->attribColor = g_yellowColor;
                    newSection->ResetColor();
                    g_canvasInstance->GetSketchShared()->AddEntity(newSection);
                    s_cache.push_back(newSection);
                    sectionPath.clear();
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
    //获取碰撞截断结果
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

bool RoughingAlgo::Intersect(const Clipper2Lib::Path64& pathA, const Clipper2Lib::Path64& pathB)
{
    bool collision = (GetIntersections(pathA, pathB).size() > 0);
    for (const Point64& vertex : pathA)
    {
        if (PointInPolygon(vertex, pathB) == PointInPolygonResult::IsInside)
        {
            collision = true;
        }
    }
    return collision;
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

double RoughingAlgo::GetDistance(const Point64& p1, const Point64& p2)
{
	double dx = static_cast<double>(p1.x - p2.x);
	double dy = static_cast<double>(p1.y - p2.y);

	return dx * dx + dy * dy;
}

void RoughingAlgo::ProbePath(const Point64& start, const Point64& end, const Path64& collisionShape,int probeDistance, std::vector<Point64>& intermidate, int deepth)
{
    //RRT 生成从起点到终点的路径
    Point64 random = start;
    bool valid = false;
    bool collision = true;
    int dist = probeDistance;
    int maxIter = 0;
    do {
		random = GenRandomPointInCircle(start.x,start.y, dist);
        if (GetDistance(end, random) < (dist * dist))
        {
            intermidate.emplace_back(random.x,random.y);
            return;
        }
        if (maxIter > 10)
        {
            dist = dist * 1.1;
            maxIter = 0;
        }
        Path64 marchingline;
		marchingline.emplace_back(start.x, start.y);
		marchingline.emplace_back(random.x, random.y);
        valid = (GetDistance(random, end) < GetDistance(start, end));
        collision = Intersect(marchingline, collisionShape);
        maxIter++;
    } while (!valid || collision);
    if (valid && !collision)
    {
        intermidate.emplace_back(random.x, random.y);
    }
    ProbePath(random,end,collisionShape,probeDistance,intermidate,deepth+1);
}

void RoughingAlgo::TrimPath(std::vector<Point64>& path, const Path64& collisionShape)
{
    //修剪多余的中间点
    Point64 startPos = path[0];
    Point64 endPos = path.back();
    int size = path.size();
    std::vector<Point64> temp;
    temp.push_back(startPos);

    int indexProbe = 1;
    while (indexProbe < size)
    {
        for (int i = indexProbe; i < size; i++)
        {
            if (Intersect({startPos,path[indexProbe]},collisionShape))
            {
                startPos = path[indexProbe - 1];
                temp.push_back(path[indexProbe-1]);
            }
            indexProbe++;
        }
        if (indexProbe < size)
        {
            temp.push_back(path[indexProbe]);
        }
    }
    temp.push_back(endPos);
    path = temp;
}

Point64 RoughingAlgo::GenRandomPointInCircle(int xc, int yc, int r)
{
    static std::random_device rd;
	static std::mt19937 gen(rd());

    static std::uniform_real_distribution<double> dis(0.0, 2.0 * PI);
    double data = dis(gen);

	double x = xc + r * cos(data);
	double y = yc + r * sin(data);

    return Point64(x, y);
}
