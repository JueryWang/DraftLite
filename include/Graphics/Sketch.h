#pragma once
#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <unordered_map>
#include "Graphics/AABB.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Canvas.h"
#include "Graphics/OCS.h"
#include "Common/ProgressInfo.h"
#include "Common/CraftParamConfig.h"
#include "UI/ArrayGenerationDlg.h"
#include "Path/Path.h"
#include "glm/glm.hpp"
#include <boost/geometry/index/rtree.hpp>

namespace CNCSYS
{
	class CanvasGPU;
	class CanvasCPU;

	class SketchGPU : public QObject
	{
		friend class OCSGPU;
		friend class CanvasGPU;
		friend class EntGroup;
		friend class EntRingConnection;

	public:
		SketchGPU();
		~SketchGPU();

		void AddEntity(EntityVGPU* e);
		void AddEntityGroup(EntGroup* group);
		void EraseEntity(EntityVGPU* e);
		void EraseEntityGroup(EntGroup* group);
		void UpdateEntityBox(EntityVGPU* e, const AABB& newbox);
		std::vector<EntityVGPU*> GetSelectedEntities();
		std::vector<EntGroup*> GetEntityGroups() { return entityGroups; }
		std::set<EntRingConnection*> GetParentRings(const std::vector<EntityVGPU*>& entities);
		void ClearEntities();
		void AddPath(Path2D* p);
		void UpdateSketch();
		void UpdateGCode(bool sendToFtp = true);
		void SetOrigin(const glm::vec3& origin);
		void GenEnvolop(float expandValue, float smoothValue);
		void SetWorkSpaceDimension(const AABB& dimension) { SimulateStatus.platformSize = dimension; }
		void SetContext(const SimulateStatus& context) { SimulateStatus = context; }
		std::string ToNcProgram();
		std::vector<EntityVGPU*> QueryBatchSelection(const AABB& box);
		EntityVGPU* QueryNearsetEntity(const glm::vec3& center, float precision);
		double GetDistanceToSelectedItems(const glm::vec3& center);
		std::tuple<EntityVGPU*, int, glm::vec2> QueryNearestPoint(const glm::vec3& center, float percision);
		CanvasGPU* GetCanvas() { return mainCanvas; }

		std::vector<EntityVGPU*> GetEntities() { return entities; }

	public slots:
		void GenRectArray(RectArrayParam param);
		void GenRingArray(RingArrayParam param);

	public:
		CanvasGPU* mainCanvas = nullptr;
		std::string source;
		CraftConfigItems attachedConfig;
		OCSGPU* attachedOCS = nullptr;

	private:
		bgi::rtree<rtree_entry, bgi::quadratic<16>> rtree;
		EntityType entType;
		std::map<int, EntityVGPU*> entityIdsMap;

		std::vector<EntityVGPU*> entities;
		std::vector<EntGroup*> entityGroups;
		std::vector<Path2D*> paths;
		SimulateStatus SimulateStatus;
		glm::vec3 wororigin = glm::vec3(0.0f);
	};

	class SketchCPU
	{
		friend class OCSCPU;
		friend class CanvasCPU;
	public:
		SketchCPU();
		~SketchCPU();

		void AddEntity(EntityVCPU* e);
		void EraseEntity(EntityVCPU* e);
		void ClearEntities();
		void UpdateSketch();
		void SplitPart();
		void SimplifyGeometry(float delta);
		void SmoothGeometry();
		void GenEnvolop(int expandValue, int smoothValue);
		void ToNcProgram(const std::string& file);
		CanvasCPU* GetCanvas() { return mainCanvas; }

	private:
		EntityType entType;
		CanvasCPU* mainCanvas;
		std::map<CanvasCPU*, EntRingConnection*> partPreview;
		std::vector<EntRingConnection*> parts;
		std::vector<EntityVCPU*> entities;
		std::vector<Polyline2DCPU*> envolopProfile;
	};
}
