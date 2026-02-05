#include "Common/GCodeHelper.h"
#include "Graphics/Primitives.h"
#define CONNECT_EPSILON 0.05

std::string GenGodeByPath(Path2D* path, SimulateStatus* Mstatus, const glm::mat4& baseMat)
{
	std::string section;
	char buffer[256];

	for (auto& pair : path->reachedEntity)
	{
		glm::mat4 transformedMatrix = MathUtils::scaledMatrix(baseMat, {Mstatus->zoom,Mstatus->zoom,Mstatus->zoom},Mstatus->wcsAnchor);
		transformedMatrix = MathUtils::tranlatedMatrix(transformedMatrix, -Mstatus->wcsAnchor);
		auto nodes = pair.first->GetTransformedNodes();
		glm::vec3 start = transformedMatrix * glm::vec4(nodes[0],1.0f);
		glm::vec3 end = transformedMatrix * glm::vec4(nodes[nodes.size() - 1], 1.0f);
		
		for (glm::vec3& node : nodes)
		{
			glm::vec3 transformed = transformedMatrix * glm::vec4(node, 1.0f);
			//Ê¹ÓÃG00
			if (pair.second)
			{
				sprintf(buffer, "N%03d G00 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
				section += buffer;
			}
			else
			{
				sprintf(buffer, "N%03d G01 X%f Y%f\n", Mstatus->ncstep++, transformed.x, transformed.y);
				section += buffer;
			}
		}
	}
	return section;
}
