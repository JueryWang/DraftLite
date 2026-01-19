#pragma once
#include <utility>
#include "Common/MathUtils.h"
#include "Graphics/AABB.h"
#include "glm/glm.hpp"

namespace CNCSYS
{
	class Shader;
	class CanvasGPU;
	class EntityRotateModal;
	class EntityScaleModal;
	class EntityMirrorModal;
	class EntityShiftModal;
	class MeasureDimensionModal;
	class SectionConfigModal;

	enum class ModalState : uint64_t
	{
		NormalInteract = 1ULL << 0,
		CreatePoint = 1ULL << 1,
		CreateLine = 1ULL << 2,
		CreatePolyline = 1ULL << 3,
		CreateCircle = 1ULL << 4,
		CreateArc = 1ULL << 5,
		CreateRectangle = 1ULL << 6,
		CreateSpline = 1ULL << 7,
		EntityMove = 1ULL << 8,
		EntityZoom = 1ULL << 9,
		EntityMirror = 1ULL << 10,
		EntityRotate = 1ULL << 11,
		EntityScale = 1ULL << 12,
		EntityShift = 1ULL << 13,
		MeasureDimension = 1ULL << 14
	};

	union ModalEventDrawFunc
	{
		void (*ptr_RotateEntity)(EntityRotateModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
		void (*ptr_ScaleEntity)(EntityScaleModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
		void (*ptr_MirrorEntity)(EntityMirrorModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
		void (*ptr_ShiftEntity)(EntityShiftModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
		void (*ptr_MeasureDimension)(MeasureDimensionModal*, CanvasGPU*, const glm::vec2&, const glm::mat4&, const glm::mat4&);
	};

	struct RotateTask_param
	{
		RotateTask_param() = default;

		glm::vec3 base = glm::vec3(0.0f);
		glm::vec3 startPoint = glm::vec3(0.0f);
		glm::vec3 endPoint = glm::vec3(0.0f);
		float deltaAngle = 0.0f;
		float angle = 0.0f;
	};

	struct ScaleTask_param
	{
		ScaleTask_param() = default;

		float scale;
		AABB selectionBox;
		glm::vec3 base;
		glm::vec3 mouse;
	};

	struct MirrrorTask_param
	{
		MirrrorTask_param() = default;

		glm::vec3 start;
		glm::vec3 end;
	};

	struct MeasureDimensionTask_param
	{
		MeasureDimensionTask_param() = default;

		glm::vec2 point1;
		glm::vec2 point2;
	};

	union ModalTaskParam
	{
		RotateTask_param* rotateTask = nullptr;
		ScaleTask_param* scaleTask;
		MirrrorTask_param* mirrorTask;
		MeasureDimensionTask_param* measureTask;
	};

	struct ModalDrawEvent
	{
		ModalState state;
		ModalEventDrawFunc modalFunc;
		ModalTaskParam param;
		int processStep = 0;
	};
}