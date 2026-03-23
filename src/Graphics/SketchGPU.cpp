#include <iostream>
#include <algorithm>
#include <random>
#include "Graphics/Anchor.h"
#include "Graphics/Sketch.h"
#include "Graphics/OCS.h"
#include "Common/MathUtils.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Graphics/Canvas.h"
#include "Graphics/Sketch.h"
#include "clipper2/clipper.h"
#include "Controls/GCodeController.h"
#include "Controls/SchedulerTask.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "UI/GCodeEditor.h"
#include "UI/MainLayer.h"
#include "UI/Configer/RoughingConfig.h"
#include "UI/Configer/WorkBlankConfig.h"
#include "IO/Utils.h"
#include "IO/DxfProcessor.h"
#include "Common/ProgressInfo.h"
#include "Algorithm/PartClassifier.h"
#include "Algorithm/RingDetector.h"
#include <QDir>

namespace CNCSYS
{
	SketchGPU::SketchGPU()
	{
	}

	SketchGPU::~SketchGPU()
	{
		ClearEntities();
	}

	void SketchGPU::AddEntity(EntityVGPU* e)
	{
		if (e != nullptr)
		{
			auto find = std::find(entities.begin(), entities.end(), e);
			if (find == entities.end())
			{
				e->SetHoldCanvas(mainCanvas);
				e->UpdatePaintData();
				bg::model::box<BoostPoint> bbox;
				bg::envelope(e->boostPath, bbox);
				rtree.insert({ bbox,e->id });
				entities.push_back(e);
				entityIdsMap[e->id] = e;
			}
			else
			{
				auto temp = e;
			}
		}
	}

	void SketchGPU::AddEntityGroup(EntGroup* group)
	{
		static std::mt19937 engine;
		static std::random_device rd;
		static std::map<int, glm::vec4> colorMap;
		engine.seed(rd());

		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		entityGroups.push_back(group);



		for (EntRingConnection* ring : group->rings)
		{
			for (EntityVGPU* comp : ring->conponents)
			{
				//if (!colorMap.count(ring->depth)) {
				//	colorMap[ring->depth] = glm::vec4(r, g, b, 1.0f);
				//}
				float r = dist(engine); // 用float更符合glm::vec4的类型
				float g = dist(engine);
				float b = dist(engine);
				comp->color = glm::vec4(r,g,b,1.0f);
				comp->ringParent = ring;
				AddEntity(comp);
			}
			ring->groupParent = group;
		}
	}

	void SketchGPU::EraseEntity(EntityVGPU* e)
	{
		entityIdsMap.erase(e->id);
		RTree_erase_by_id(rtree, e->id);

		auto find = std::find(entities.begin(), entities.end(), e);
		if (find != entities.end())
		{
			entities.erase(find);
		}
		try
		{
			if (e->ringParent != nullptr)
			{
				EntRingConnection* ring = e->ringParent;
				ring->EraseEntity(e, this);
			}
		}
		catch (...)
		{
			std::cout << "Exec Error"<<__FUNCTIONW__ << std::endl;
		}
		e = nullptr;
	}

	void SketchGPU::EraseEntityGroup(EntGroup* group)
	{

	}

	void SketchGPU::UpdateEntityBox(EntityVGPU* e, const AABB& newbox)
	{
		RTree_erase_by_id(rtree, e->id);
		bg::model::box<BoostPoint> bbox(BoostPoint(newbox.min.x, newbox.min.y), BoostPoint(newbox.max.x, newbox.max.y));
		rtree.insert({ bbox,e->id });
	}

	std::vector<EntityVGPU*> SketchGPU::GetSelectedEntities()
	{
		return mainCanvas->selectedItems;
	}

	std::set<EntRingConnection*> SketchGPU::GetParentRings(const std::vector<EntityVGPU*>& entities)
	{
		std::set<EntRingConnection*> groups;
		for (EntityVGPU* ent : entities)
		{
			if (ent->ringParent != NULL)
			{
				groups.insert(ent->ringParent);
			}
		}
		return groups;
	}

	void SketchGPU::ClearEntities()
	{
		for (Path2D* path : paths)
		{
			delete path;
		}
		for (EntGroup* group : entityGroups)
		{
			delete group;
		}
		EntityVGPU::counter = 0;

		rtree.clear();
		entities.clear();
		entityGroups.clear();
		entityIdsMap.clear();
		paths.clear();
		GCodeController::GetController()->CleanCache();
		GCodeEditor::GetInstance()->CleanCache();
		g_MScontext.ncstep = 0;
		if (mainCanvas)
		{
			mainCanvas->ResetCanvas();
		}

		WorkBlankConfigPage::s_attachedRing = nullptr;
	}

	void SketchGPU::AddPath(Path2D* p)
	{
		paths.push_back(p);
	}

	void SketchGPU::ReOrganize()
	{
		for (EntGroup* group : entityGroups)
		{
			delete group;
		}
		std::vector<EntRingConnection*> rings = RingDetector::RingDetect(entities);
		PartClassifier classifier(rings);
		std::vector<EntGroup*> groups = classifier.Execute();

		for (EntGroup* group : groups)
		{
			AddEntityGroup(group);
		}
		UpdateSketch();

		for (EntGroup* group : groups)
		{
			for (EntRingConnection* ring : group->rings)
			{
				if (ring->direction == GeomDirection::CW)
				{
					ring->Reverse();
					ring->direction = GeomDirection::CCW;
				}
			}
		}

		SetOrigin(mainCanvas->GetOCSSystem()->objectRange->getMin());
	}

	void SketchGPU::UpdateSketch()
	{
		if (mainCanvas)
		{
			mainCanvas->UpdateOCS();
		}
		GCodeController::GetController()->CleanCache();
		GCodeEditor::GetInstance()->CleanCache();
	}

	void SketchGPU::UpdateGCode(bool sendToFtp)
	{
		GCodeController::GetController()->CleanCache();
		GCodeEditor::GetInstance()->CleanCache();
		QString ncProgram = QString::fromStdString(ToNcProgram());
		GCodeEditor::GetInstance()->setText(ncProgram);

		if (g_opcuaClient && sendToFtp)
		{
			SCT_SEQUENCE_TASK* updateGCode = new SCT_SEQUENCE_TASK();
			QString title = QString::fromLocal8Bit(source.c_str());
			QStringList parts = title.split("/");
			QString fileName = FileNameFromPath(parts.last()) + ".cnc";
			updateGCode->params.updateRemoteFtp = new TaskUpdateRemoteFtpParam();
			updateGCode->params.updateRemoteFtp->fileUrl = "Share_files_anonymity/" + fileName.toLocal8Bit();
			updateGCode->type = SCT_TASK_TYPE::UPDATE_FTP_GCODE;
			ScadaScheduler::GetInstance()->AddTask(updateGCode);
		}
	}

	void SketchGPU::SetOrigin(const glm::vec3& origin)
	{
		for (EntGroup* group : entityGroups)
		{
			for (EntRingConnection* ring : group->rings)
			{
				ring->Move(-origin);
			}
		}

		for (EntityVGPU* ent : entities)
		{
			this->UpdateEntityBox(ent, ent->bbox);
		}
		attachedOCS->ComputeScaleFitToCanvas();
		this->UpdateGCode(true);
	}

	void SketchGPU::GenRectArray(RectArrayParam arrayParam)
	{
		std::vector<EntityVGPU*> selectedItems = mainCanvas->GetSelectedEntitys();
		for (int i = 0; i < arrayParam.rowCount; i++)
		{
			for (int j = 0; j < arrayParam.colCount; j++)
			{
				for (EntityVGPU* ent : selectedItems)
				{
					if (i == 0 && j == 0)
						continue;
					EntityVGPU* newEnt = ent->Clone();
					float offsetX = (arrayParam.colDir == ColDirection::LEFT ? -j * arrayParam.offsetCol : j * arrayParam.offsetCol);
					float offsetY = (arrayParam.rowDir == RowDirection::UP ? i * arrayParam.offsetRow : -i * arrayParam.offsetRow);
					glm::vec3 offset = glm::vec3(offsetX, offsetY, 0.0f);
					newEnt->Move(offset);
					AddEntity(newEnt);
				}
			}
		}
	}

	void SketchGPU::GenRingArray(RingArrayParam ringParam)
	{
		glm::vec3 RotCenter;
		glm::vec3 worldCentroid = glm::vec3(0.0f);

		std::vector<EntityVGPU*> selectedItems = mainCanvas->GetSelectedEntitys();
		for (EntityVGPU* ent : selectedItems)
		{
			worldCentroid += ent->GetTransformedCentroid();
		}
		worldCentroid /= selectedItems.size();

		if (ringParam.setCenterParam)
		{
			RotCenter.x = worldCentroid.x + ringParam.radius * sin(ringParam.startupAngle);
			RotCenter.y = worldCentroid.y + ringParam.radius * cos(ringParam.startupAngle);
		}

		float rotationStep;
		if (ringParam.RingType == RingArrayType::BaseOnSpacing)
		{
			rotationStep = ringParam.angleSpacing;
		}
		else
		{
			rotationStep = ringParam.angleRange / ringParam.itemCount;
		}

		float currentAngle = ringParam.startupAngle;
		for (int i = 0; i < ringParam.itemCount; i++)
		{
			for (EntityVGPU* ent : selectedItems)
			{
				EntityVGPU* newEnt = ent->Clone();
				newEnt->Rotate(RotCenter, currentAngle);
				AddEntity(newEnt);
			}
			currentAngle += rotationStep;
		}
	}

	void SketchGPU::GenEnvolop(float expandValue, float smoothValue)
	{

	}

	std::string SketchGPU::ToNcProgram()
	{
		std::string content;
		auto groups = GetEntityGroups();
		g_MScontext.totalPath = 0;
		g_MScontext.idlePath = 0;
		if (groups.size())
		{
			std::sort(entityGroups.begin(), entityGroups.end(), [&](EntGroup* g1, EntGroup* g2) {return g1->processOrder < g2->processOrder; });
			g_MScontext.ncstep = 0;
			g_MScontext.toolPos = glm::vec3(0, 0, 0);
			for (EntGroup* group : entityGroups)
			{
				std::sort(group->rings.begin(), group->rings.end(), [&](EntRingConnection* r1, EntRingConnection* r2) { return r1->processOrder < r2->processOrder; });
				for (EntRingConnection* ring : group->rings)
				{
					content += ring->ToNcInstruction(&g_MScontext, true, this);
				}
			}
		}
		keyparams.dimensionWidth = attachedOCS->objectRange->XRange();
		keyparams.dimensionHeight = attachedOCS->objectRange->YRange();
		keyparams.pathLength = g_MScontext.totalPath;
		keyparams.idleLength = g_MScontext.idlePath;
		//Anchor::GetInstance()->ReAssignDataSize(g_MScontext.ncstep);
		return content;
	}

	std::vector<EntityVGPU*> SketchGPU::QueryBatchSelection(const AABB& box)
	{
		std::vector<rtree_entry> entries;
		bg::model::box<BoostPoint> bbox(BoostPoint(box.min.x, box.min.y), BoostPoint(box.max.x, box.max.y));
		rtree.query(bgi::intersects(bbox), std::back_inserter(entries));
		std::vector<EntityVGPU*> filteredItems;
		for (const auto& entry : entries)
		{
			filteredItems.push_back(entityIdsMap[entry.second]);
		}
		std::vector<EntityVGPU*> res;
		//筛选完成后再基于线做相交性判断
		for (EntityVGPU* ent : filteredItems)
		{
			if (ent && ent->isVisible)
			{
				std::vector<glm::vec3> nodes = ent->GetTransformedNodes();
				for (int i = 0; i < nodes.size() - 1; i++)
				{
					if (box.Intersect(nodes[i], nodes[i + 1]))
					{
						res.push_back(ent);
						break;
					}
				}
			}
		}

		return res;
	}

	EntityVGPU* SketchGPU::QueryNearsetEntity(const glm::vec3& center, float captureRadius)
	{
		EntityVGPU* res = nullptr;
		std::vector<EntityVGPU*> filteredEntity = QueryBatchSelection(AABB(center - glm::vec3(captureRadius, captureRadius, 0.0f), center + glm::vec3(captureRadius, captureRadius, 0.0f)));

		if (filteredEntity.size() == 1)
		{
			return filteredEntity[0];
		}

		double minDistance = std::numeric_limits<double>::max();

		for (int i = 0; i < filteredEntity.size(); i++)
		{
			double d = distance_to_polygon_boundary(BoostPoint(center.x, center.y), filteredEntity[i]->boostPath);

			if (d < minDistance)
			{
				minDistance = d;
				res = filteredEntity[i];
			}
		}

		return res;
	}

	double SketchGPU::GetDistanceToSelectedItems(const glm::vec3& center)
	{
		double min_distance = std::numeric_limits<double>::max();
		BoostPoint target = BoostPoint(center.x, center.y);
		for (EntityVGPU* ent : entities)
		{
			if (ent->isSelected)
			{
				double distance = distance_to_polygon_boundary(target, ent->boostPath);
				if (distance < min_distance)
					min_distance = distance;
			}
		}

		return min_distance;
	}

	std::tuple<EntityVGPU*, int, glm::vec2>  SketchGPU::QueryNearestPoint(const glm::vec3& center, float captureRadius)
	{
		glm::vec2 nearestPointPosition;
		std::vector<EntityVGPU*> filteredEntity;
		std::vector<rtree_entry> entries;
		AABB intersectBox = AABB(center - glm::vec3(captureRadius, captureRadius, 0.0f), center + glm::vec3(captureRadius, captureRadius, 0.0f));
		bg::model::box<BoostPoint> bbox(BoostPoint(intersectBox.min.x, intersectBox.min.y), BoostPoint(intersectBox.max.x, intersectBox.max.y));
		rtree.query(bgi::intersects(bbox), std::back_inserter(entries));
		for (const auto& entry : entries)
		{
			filteredEntity.push_back(entityIdsMap[entry.second]);
		}

		EntityVGPU* findEntity;
		int findIndex;
		double minDistance = std::numeric_limits<double>::max();
		BoostPoint targetSearch(center.x, center.y);
		BoostPoint result;

		for (int i = 0; i < filteredEntity.size(); i++)
		{
			if (filteredEntity[i]->isVisible)
			{
				float distance;
				double distanceFromStart = bg::distance(targetSearch, filteredEntity[i]->boostPath[0]);
				double distanceFromEnd = bg::distance(targetSearch, filteredEntity[i]->boostPath.back());
				if (distanceFromStart < distanceFromEnd)
				{
					minDistance = distanceFromStart;
					result = filteredEntity[i]->boostPath[0];
					findEntity = filteredEntity[i];
					findIndex = 0;
				}
				else
				{
					minDistance = distanceFromEnd;
					result = filteredEntity[i]->boostPath.back();
					findEntity = filteredEntity[i];
					findIndex = filteredEntity[i]->boostPath.size()-1;
				}
			}
		}
		if (minDistance < captureRadius)
			return std::tuple<EntityVGPU*, int, glm::vec2>(findEntity, findIndex, glm::vec2(result.x(), result.y()));

		return std::tuple<EntityVGPU*, int, glm::vec2>(nullptr, -1, glm::vec2(0, 0));
	}

	void SketchGPU::SetCanvas(CanvasGPU* canvas)
	{
		mainCanvas = canvas;
		attachedOCS = canvas->GetOCSSystem();
	}
}

