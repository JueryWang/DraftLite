#include "Algorithm/RingDetector.h"
//小于这个值拼接两段
#define CONNECTION_TOLERANCE 1e-3

RingDetector::RingDetector(const std::vector<EntityVGPU*> entities)
{
}

std::vector<EntRingConnection*> RingDetector::RingDetect(const std::vector<EntityVGPU*> entities)
{

	std::vector<std::list<RingNode*>> ringGroups;
	std::set<RingNode*> nodeToDelete;

	for (int i = 0; i < entities.size(); i++)
	{
		EntityVGPU* currentEnt = entities[i];
		std::vector<glm::vec3> nodes = currentEnt->GetTransformedNodes();
		glm::vec3 start = nodes[0];
		glm::vec3 end = nodes.back();

		RingNode* group = new RingNode();
		nodeToDelete.insert(group);

		group->leftBound = start;
		group->rightBound = end;
		if (currentEnt->GetType() != EntityType::Line && currentEnt->direction != GeomDirection::CCW)
		{
			currentEnt->Reverse();
			std::swap(group->leftBound, group->rightBound);
		}
		group->entity = currentEnt;
		std::list<RingNode*> ringlist;
		ringlist.push_front(group);
		ringGroups.push_back(ringlist);
	}

	for (int i = 0; i < 2; i++)
	{
		//正向执行一次合并
		int lastGroupSize = ringGroups.size();
		do
		{
			lastGroupSize = ringGroups.size();
			int eraseCount = 0;

			for (int i = 0; i < (lastGroupSize - eraseCount); i++)
			{
				for (int j = i + 1; j < (lastGroupSize - eraseCount); j++)
				{
					RingNode* groupIBegin = ringGroups[i].front();
					RingNode* groupIEnd = ringGroups[i].back();
					RingNode* groupJBegin = ringGroups[j].front();
					RingNode* groupJEnd = ringGroups[j].back();

					if (glm::distance(groupJEnd->rightBound, groupIBegin->leftBound) < CONNECTION_TOLERANCE)
					{
						if (groupJEnd->entity->id != groupIBegin->entity->id)
						{
							while (!ringGroups[j].empty())
							{
								auto back = ringGroups[j].back();
								ringGroups[i].push_front(back);
								ringGroups[j].pop_back();
							}
							std::swap(ringGroups[j], *(ringGroups.end() - (eraseCount + 1)));
							eraseCount++;
							continue;
						}
					}
					else if (glm::distance(groupJBegin->leftBound, groupIEnd->rightBound) < CONNECTION_TOLERANCE)
					{
						if (groupJBegin->entity->id != groupIEnd->entity->id)
						{
							while (!ringGroups[j].empty())
							{
								auto begin = ringGroups[j].front();
								ringGroups[i].push_back(begin);
								ringGroups[j].pop_front();
							};
							std::swap(ringGroups[j], *(ringGroups.end() - (eraseCount + 1)));
							eraseCount++;
							continue;
						}
					}
				}
			}
			ringGroups.resize(lastGroupSize - eraseCount);
		} while (lastGroupSize != ringGroups.size());

		//反向执行一次
		do
		{
			lastGroupSize = ringGroups.size();
			int eraseCount = 0;

			for (int i = 0; i < (lastGroupSize - eraseCount); i++)
			{
				for (int j = i + 1; j < (lastGroupSize - eraseCount); j++)
				{
					RingNode* groupIBegin = ringGroups[i].front();
					RingNode* groupIEnd = ringGroups[i].back();
					RingNode* groupJBegin = ringGroups[j].front();
					RingNode* groupJEnd = ringGroups[j].back();

					if (glm::distance(groupJEnd->rightBound, groupIEnd->rightBound) < CONNECTION_TOLERANCE)
					{
						if (groupJEnd->entity->id != groupIEnd->entity->id)
						{
							while (!ringGroups[j].empty())
							{
								auto back = ringGroups[j].back();
								back->entity->Reverse();
								std::swap(back->leftBound, back->rightBound);
								ringGroups[i].push_back(back);
								ringGroups[j].pop_back();
							}
							std::swap(ringGroups[j], *(ringGroups.end() - (eraseCount + 1)));
							eraseCount++;
							continue;
						}
					}
					else if (glm::distance(groupJBegin->leftBound, groupIBegin->leftBound) < CONNECTION_TOLERANCE)
					{
						if (groupJBegin->entity->id != groupIBegin->entity->id)
						{
							while (!ringGroups[j].empty())
							{
								auto front = ringGroups[j].front();
								front->entity->Reverse();
								std::swap(front->leftBound, front->rightBound);
								ringGroups[i].push_front(front);
								ringGroups[j].pop_front();
							}
							std::swap(ringGroups[j], *(ringGroups.end() - (eraseCount + 1)));
							eraseCount++;
							continue;
						}
					}
				}
			}
			ringGroups.resize(lastGroupSize - eraseCount);
		} while (lastGroupSize != ringGroups.size());
	}

	std::vector<EntRingConnection*> res;
	for (auto& group : ringGroups)
	{
		std::vector<EntRingConnection> ringConn;

		EntRingConnection* pre = nullptr;

		std::vector<EntityVGPU*> groupConponents;
		for (auto& node : group)
		{
			groupConponents.push_back(node->entity);
		}

		EntRingConnection* compound = new EntRingConnection(groupConponents);
		res.push_back(compound);
	}

	for (RingNode* node : nodeToDelete)
	{
		delete node;
	}

	return res;
}
