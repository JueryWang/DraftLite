#include "Algorithm/RoughingAlgo.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "Graphics/OCS.h"
#include "UI/GLWidget.h"
#include "Controls/GCodeController.h"
#include <list>
#include <map>
#include <chrono>
#include <random>
#include <QPoint>
#include <QApplication>
#include "Common/ClipperFuncWrapper.h"
#include "ModalEvent/EvSendCanvasTag.h"
#include "Algorithm/RRT.h"
#include "Path/Path.h"

std::vector<CNCSYS::EntityVGPU*> RoughingAlgo::s_cache;
std::map<int, std::vector<PointClusterNode>> RoughingAlgo::regionSet;
std::map<int, Path2D*> RoughingAlgo::regionPaths;
int RoughingAlgo::percision;
EntRingConnection* lastExecShape = nullptr;

std::string RoughingAlgo::GetRoughingPath(EntRingConnection* shape, AABB& workblank, RoughingParamSettings setting)
{
    lastExecShape = shape;
    if (RoughingAlgo::s_cache.size())
    {
        for (EntityVGPU* path : RoughingAlgo::s_cache)
        {
            g_canvasInstance->GetSketchShared()->EraseEntity(path);
            delete path;
        }
        RoughingAlgo::s_cache.clear();
        for (auto& pair : regionPaths)
        {
            delete pair.second;
        }
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
                //sectionLine->attribColor = g_yellowColor;
                //sectionLine->ResetColor();
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
    regionSet = PointRegionCluster::kmeans(clusterNodes, clusterInitCenter, clusterInitCenter.size(), 50);

    for (auto& pair : regionSet)
    {
        //由内往外翻转为由外向内
        std::reverse(pair.second.begin(), pair.second.end());
        glm::vec4 randomColor = GetRandomColor();
        glm::vec3 regionCentroid = glm::vec3(0, 0, 0);
        for (PointClusterNode& pNode : pair.second)
        {
            pNode.entityParent->attribColor = randomColor;
            pNode.entityParent->ResetColor();
            pNode.regionSetting.rotAnchor = workblank.Center();
            regionCentroid += pNode.entityParent->centroid;
        }
        regionCentroid /= pair.second.size();

        EvSendCanvasTag* EvclusterTag = new EvSendCanvasTag(regionCentroid, QString::number(pair.first), 20);
        QApplication::postEvent(g_canvasInstance->GetFrontWidget(), EvclusterTag, Qt::HighEventPriority);
    }

    bool toolInited = false;
    glm::vec3 lastEnd;
    char buffer[256];
    for (auto& pair : regionSet)
    {
        Path2D* path = new Path2D();
        regionPaths[pair.first] = path;
        std::vector<glm::vec3> sectionPath;

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
                    Point64 nearestPt;
                    double nearestDist = DBL_MAX;
                    for (Point64& pt : involute_sequence[0])
                    {
                        double dist = GetDistance(marchingline[0], pt);
                        if (dist < nearestDist)
                        {
                            nearestDist = dist;
                            nearestPt = pt;
                        }
                    }
                    std::vector<glm::vec3> probePts;
                    std::vector<Point64> probePtsPoint64;
                    probePts.push_back(lineS);
                    probePts.push_back({ (float)nearestPt.x / RoughingAlgo::percision,(float)nearestPt.y / RoughingAlgo::percision,0.0f });
                    int depth = 0;
                    ProbePath(nearestPt, marchingline[1], involute_sequence[0], setting.stepover* RoughingAlgo::percision, probePtsPoint64, depth);
                    TrimPath(probePtsPoint64, involute_sequence[0]);
                    for (Point64& pt : probePtsPoint64)
                    {
                        probePts.push_back({ (float)pt.x / RoughingAlgo::percision,(float)pt.y / RoughingAlgo::percision ,0.0f });
                    }
                    probePts.push_back(lineEd);
                    {
                        Polyline2DGPU* probePoly = new Polyline2DGPU();
                        probePoly->SetParameter(probePts, false);
                        probePoly->attribColor = g_redColor;
                        probePoly->ResetColor();
                        g_canvasInstance->GetSketchShared()->AddEntity(probePoly);
                        s_cache.push_back(probePoly);

                        char buffer[256];
                        for (int i = 0; i < probePts.size(); i++)
                        {
                            std::sprintf(buffer, "N%03d G00 X%f Y%f \n", g_MScontext.ncstep++, probePts[i].x - g_MScontext.wcsAnchor.x, probePts[i].y - g_MScontext.wcsAnchor.y);
                            gcode += buffer;
                        }
                    }

                    Polyline2DGPU* jumpline = new Polyline2DGPU();
                    jumpline->SetParameter(probePts,false);
                    jumpline->attribColor = g_redColor;
                    jumpline->ResetColor();
                    g_canvasInstance->GetSketchShared()->AddEntity(jumpline);
                    s_cache.push_back(jumpline);
                    //regionPaths[pair.first]->AddNext(jumpline,true);
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
                    for (Point64& pt : involute_sequence[std::max(collisionLayer-1,0)])
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
                    TrimPath(probePtsPoint64, involute_sequence[collisionLayer]);
                    for (Point64& pt : probePtsPoint64)
                    {
                        probePts.push_back({(float)pt.x/ RoughingAlgo::percision,(float)pt.y/ RoughingAlgo::percision ,0.0f});
                    }
                    probePts.push_back(lineEd);

                    Polyline2DGPU* probePoly = new Polyline2DGPU();
                    probePoly->SetParameter(probePts, false);
                    probePoly->attribColor = g_redColor;
                    probePoly->ResetColor();
                    g_canvasInstance->GetSketchShared()->AddEntity(probePoly);
                    s_cache.push_back(probePoly);

                    char buffer[256];
                    for (int i = 0; i < probePts.size(); i++)
                    {
                        std::sprintf(buffer,"N%03d G00 X%f Y%f \n",g_MScontext.ncstep++, probePts[i].x - g_MScontext.wcsAnchor.x, probePts[i].y - g_MScontext.wcsAnchor.y);
                        gcode += buffer;
                    }
                    regionPaths[pair.first]->AddNext(probePoly,true);

                    sectionPath.clear();
                }

            }

            lastEnd = nodes.back();
            layer->ResetColor();
            g_canvasInstance->GetSketchShared()->AddEntity(layer);
            s_cache.push_back(layer);
            gcode += layer->GenNcSection(&g_MScontext, false);
            if (!sectionPath.size())
            {
               regionPaths[pair.first]->AddNext(layer,false);
            }
            sectionPath.insert(sectionPath.end(), nodes.begin(), nodes.end());
            toolInited = true;
        }
    }

    delete originShape;
    delete offsetAllowance;

    return gcode;
}

std::string RoughingAlgo::RequestRegion(const std::vector<int>& groupNumbers,RegionParamSettings setting)
{
    std::string gcode;

    bool toolInited = false;
    glm::vec3 lastEnd;
    float rotateAngle = setting.rotation;
    
    if (setting.dir == GeomDirection::CW)
        rotateAngle = -rotateAngle;

    float rotateDeg = glm::radians(rotateAngle);
    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f),-setting.rotAnchor);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f),rotateDeg,glm::vec3(0,0,1));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), setting.rotAnchor);

    glm::mat4 baseTransform = translateBack * rotation * translateToOrigin;

    for (int regionId : groupNumbers)
    {
        gcode += GenGodeByPath(regionPaths[regionId], &g_MScontext, baseTransform);
    }
    return gcode;
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
