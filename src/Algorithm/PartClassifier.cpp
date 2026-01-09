#include "Algorithm/PartClassifier.h"
#include "Algorithm/RingDetector.h"
#include <iostream>
#define PRECISION 1000

PartClassifier::PartClassifier(const std::vector<EntRingConnection*>& _rings) : rings(_rings)
{
	for (EntRingConnection* ring : rings)
	{
		Clipper2Lib::Path64* subject = new Clipper2Lib::Path64();

		for (const glm::vec3& p : ring->contour)
		{
			subject->emplace_back(p.x * PRECISION, p.y * PRECISION);
		}
		pathMapper[ring] = subject;
	}
}

PartClassifier::~PartClassifier()
{
	for (auto& pair : pathMapper)
	{
		delete pair.second;
	}
	pathMapper.clear();
}

std::vector<EntGroup*> PartClassifier::Execute()
{
	partList.clear();
	std::sort(rings.begin(), rings.end(), [](EntRingConnection* cn1, EntRingConnection* cn2) {return cn1->area > cn2->area; });
	
	for (EntRingConnection* ring : rings)
	{
		bool within = false;
		for (auto& part : partList)
		{
			float boxArea1 = part->bbox.Area();
			float boxArea2 = ring->bbox.Area();
			float minArea = std::min(boxArea1, boxArea2);
			float epsilon = minArea * 0.001f;

			float intersectArea = part->bbox.IntersectionArea(ring->bbox);

			if (part->bbox.Contains(ring->bbox))
			{
				part->AddRingConnection(ring);
				within = true;
				break;
			}
			else if (IsPolygonContains(*pathMapper[ring],*pathMapper[part->rings[0]]))
			{
				part->AddRingConnection(ring);
				within = true;
				break;
			}
		}

		if (!within)
		{
			partList.push_back(new EntGroup({ring}));
		}
	}
	
	return partList;
}

inline bool PartClassifier::IsPolygonContains(const Clipper2Lib::Path64& child, const Clipper2Lib::Path64& parent)
{
	for (const auto& pt : child) {
		if (Clipper2Lib::PointInPolygon(pt, parent) == Clipper2Lib::PointInPolygonResult::IsOutside) {
			return false;
		}
	}
	return true;
}