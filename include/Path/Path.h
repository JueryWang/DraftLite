#pragma once
#include "Graphics/DrawEntity.h"
#include <glm/glm.hpp>
#include <vector>

using namespace CNCSYS;

struct PathNode {
    glm::vec3 Node;
    glm::vec3 Normal;
    glm::vec3 Tangent;
};

//Path 和 DrawEntity的区别是Path无法在场景中交互,只能通过调用算法改变其内部结构;DrawEntity可以在场景中编辑
class Path2D
{
public:
	Path2D(EntityVGPU* ent_from);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="nodes"> 节点位置</param>
    /// <param name="isIdle">是否为空程路径</param>
    Path2D(const std::vector<glm::vec3>& nodes,bool isIdle);
	~Path2D();
    std::vector<PathNode> GetTracks() { return tracks; }
    void SetTransformation(const glm::mat4& matrix) { worldModelMatrix = matrix; }
    glm::mat4 GetTransformation() { return worldModelMatrix; }
    std::vector<glm::vec3> GetTransformedNodes();

public:
    bool visiable = false;

private:
	std::vector<PathNode> tracks;
    bool Idle = false;
    glm::mat4 worldModelMatrix;
};