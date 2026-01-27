#include "Algorithm/VisibilityGraph.h"
#include <iostream>

VisibilityGraph::VisibilityGraph()
{

}

VisibilityGraph::~VisibilityGraph()
{

}

void VisibilityGraph::addObstacle(const Path64& polygon)
{
    obstacles.push_back(polygon);
    //for (const Point64& pt : polygon)
    //{
    //    nodes.push_back(pt);
    //}
}

void VisibilityGraph::addExtraPoint(const Point64& p)
{
	nodes.push_back(p);
}

void VisibilityGraph::buildGraph(const Point64& start, const Point64& end)
{
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
        nodes.insert(nodes.begin(), end);
        nodes.insert(nodes.begin(), start);

        int n = nodes.size();
        adj.assign(n, std::vector<std::pair<int, double>>());

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (is_visible(nodes[i], nodes[j])) {
                    double d = std::hypot(nodes[i].x - nodes[j].x, nodes[i].y - nodes[j].y);
                    adj[i].push_back({ j, d });
                    adj[j].push_back({ i, d });
                }
            }
        }
#else 
        nodes.insert(nodes.begin(), end);
        nodes.insert(nodes.begin(), start);

        int n = nodes.size();
        adj.assign(n, std::vector<std::pair<int, double>>());

        // 2. 并行处理：每个线程先构建局部邻接表，最后合并
    #pragma omp parallel
        {
            // 每个线程创建局部邻接表（线程私有，无竞争）
            std::vector<std::vector<std::pair<int, double>>> local_adj(n);

            // 并行分配外层循环（取消dynamic，用默认的static更高效，任务量均衡后）
    #pragma omp for schedule(static)
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    if (is_visible(nodes[i], nodes[j])) {
                        double d = std::hypot(nodes[i].x - nodes[j].x, nodes[i].y - nodes[j].y);
                        // 操作局部数据，无锁、无竞争
                        local_adj[i].emplace_back(j, d);
                        local_adj[j].emplace_back(i, d);
                    }
                }
            }

            // 3. 单线程合并局部数据到全局adj（仅一次合并，无频繁锁）
    #pragma omp critical
            {
                for (int i = 0; i < n; ++i) {
                    adj[i].insert(adj[i].end(), local_adj[i].begin(), local_adj[i].end());
                }
            }
        }
#endif
}

void VisibilityGraph::ClearGraph()
{
    obstacles.clear();
    nodes.clear();
}

//bfs求可达路径
std::vector<glm::vec3> VisibilityGraph::solve()
{
    int n = nodes.size();
    std::vector<bool> visited(n, false); // 仅标记是否访问过，无需距离数组
    std::vector<int> parent(n, -1);      // 保留父节点用于回溯路径
    std::queue<int> q;                   // 普通队列替代优先级队列

    // 初始化：起点入队，标记已访问
    int start = 0;
    int end = 1;
    visited[start] = true;
    q.push(start);

    bool found = false; // 标记是否找到终点
    while (!q.empty() && !found) {
        int u = q.front();
        q.pop();

        // 遍历当前节点的所有邻接边
        for (auto& edge : adj[u]) {
            int v = edge.first;
            if (!visited[v]) { // 仅访问未遍历过的节点
                visited[v] = true;
                parent[v] = u; // 记录父节点
                q.push(v);

                // 找到终点，立即终止遍历（核心优化）
                if (v == end) {
                    found = true;
                    break;
                }
            }
        }
    }

    // 回溯路径（逻辑与原代码一致）
    std::vector<glm::vec3> path;
    if (found) { // 确保终点可达
        for (int curr = end; curr != -1; curr = parent[curr]) {
            path.push_back(glm::vec3(nodes[curr].x / precision, nodes[curr].y / precision, 0.0f));
        }
        std::reverse(path.begin(), path.end());
    }
    // 若终点不可达，返回空路径
    return path;
}

bool VisibilityGraph::is_visible(const Point64& p1, const Point64& p2) const
{
    Paths64 line;
    line.push_back({ p1,p2 });

    Paths64 intersect;
    Paths64 dummy;

    Clipper64 clipper;
    clipper.AddOpenSubject(line);
    clipper.AddClip(obstacles); 
    clipper.Execute(ClipType::Intersection, FillRule::NonZero, dummy, intersect);
    return intersect.empty();
}
