#include "Algorithm/VisibilityGraph.h"
#include <iostream>

VisibilityGraph::VisibilityGraph()
{

}

VisibilityGraph::~VisibilityGraph()
{

}

void VisibilityGraph::addObstacle(const std::vector<Point64>& polygon)
{
    obstacles.push_back(polygon);
	//for (const auto& p : polygon) nodes.push_back(p);
}

void VisibilityGraph::addExtraPoint(const Point64& p)
{
	nodes.push_back(p);
}

void VisibilityGraph::buildGraph(const Point64& start, const Point64& end)
{
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
}

std::vector<glm::vec3> VisibilityGraph::solve()
{
    int n = nodes.size();
    std::vector<double> dist(n, 1e18);
    std::vector<int> parent(n, -1);
    using pdi = std::pair<double, int>;
    std::priority_queue<pdi, std::vector<pdi>, std::greater<pdi>> pq;

    dist[0] = 0; // 0 是起点
    pq.push({ 0.0, 0 });

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;
        if (u == 1) break; // 到达终点

        for (auto& edge : adj[u]) {
            int v = edge.first;
            double weight = edge.second;
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.push({ dist[v], v });
            }
        }
    }

    // 回溯路径
    std::vector<glm::vec3> path;
    for (int curr = 1; curr != -1; curr = parent[curr]) {
        path.push_back(glm::vec3(nodes[curr].x / 1000.0f,nodes[curr].y/1000.0f,0.0f));
    }
    std::reverse(path.begin(), path.end());
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
