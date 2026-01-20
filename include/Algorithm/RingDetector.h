#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/functional/hash.hpp>
#include <tuple>
#include <cmath>
#include <vector>
#include <list>

struct RingNode
{
    glm::vec3 leftBound;
    glm::vec3 rightBound;
    EntityVGPU* entity;
};

//检测环
class RingDetector
{
public:

    RingDetector(const std::vector<EntityVGPU*> entities);
    ~RingDetector();

	static std::vector<EntRingConnection*> RingDetect(const std::vector<EntityVGPU*> entities);

    //针对conponent有变更的ring,尝试修复顺序关系
    static void RepairRing(EntRingConnection* ring);
};
