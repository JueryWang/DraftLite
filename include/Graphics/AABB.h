#pragma once
#include "glm/glm.hpp"
#include "Common/MathUtils.h"
#include <iostream>
#include <utility>
#include <algorithm>

namespace CNCSYS
{
	class AABB
	{
	public:
		AABB()
		{

		}
		AABB(const glm::vec3& min, const glm::vec3& max)
		{
			this->min.x = std::min(min.x, max.x);
			this->min.y = std::min(min.y, max.y);
			this->min.z = std::min(min.z, max.z);
			this->max.x = std::max(min.x, max.x);
			this->max.y = std::max(min.y, max.y);
			this->max.z = std::max(min.z, max.z);
		}

		AABB(const AABB& other)
			: min(other.min), max(other.max) {
		}

		AABB(AABB* other) : min(other->min), max(other->max) {}

	public:
		void Union(const glm::vec3& p)
		{
			min.x = std::min(min.x, p.x);
			min.y = std::min(min.y, p.y);
			min.z = std::min(min.z, p.z);
			max.x = std::max(max.x, p.x);
			max.y = std::max(max.y, p.y);
			max.z = std::max(max.z, p.z);
		}

		void Union(AABB* other)
		{
			min.x = std::min(min.x, other->min.x);
			min.y = std::min(min.y, other->min.y);
			min.z = std::min(min.z, other->min.z);
			max.x = std::max(max.x, other->max.x);
			max.y = std::max(max.y, other->max.y);
			max.z = std::max(max.z, other->max.z);
		}

		void Union(const AABB& other)
		{
			min.x = std::min(min.x, other.min.x);
			min.y = std::min(min.y, other.min.y);
			min.z = std::min(min.z, other.min.z);
			max.x = std::max(max.x, other.max.x);
			max.y = std::max(max.y, other.max.y);
			max.z = std::max(max.z, other.max.z);
		}

		bool Intersect(const AABB& other)
		{
			bool xOverlap = (min.x <= other.max.x) && (max.x >= other.min.x);
			bool yOverlap = (min.y <= other.max.y) && (max.y >= other.min.y);
			return xOverlap && yOverlap;
		}

		bool Intersect(const glm::vec3& p1, const glm::vec3& p2) const
		{
			glm::vec3 dir = p2 - p1;
			float t_min = 0.0f;
			float t_max = 1.0f;

			if (std::fabs(dir.x) < 1e-6f)
			{
				if (p1.x < min.x || p1.x > max.x)
					return false;
			}
			else
			{
				float t1 = (min.x - p1.x) / dir.x;
				float t2 = (max.x - p1.x) / dir.x;
				if (t1 > t2) std::swap(t1, t2);
				t_min = std::max(t_min, t1);
				t_max = std::min(t_max, t2);
				if (t_min > t_max)
					return false;
			}

			if (std::fabs(dir.y) < 1e-6f)
			{
				if (p1.y < min.y || p1.y > max.y)
					return false;
			}
			else
			{
				float t1 = (min.y - p1.y) / dir.y;
				float t2 = (max.y - p1.y) / dir.y;
				if (t1 > t2) std::swap(t1, t2);
				t_min = std::max(t_min, t1);
				t_max = std::min(t_max, t2);
				if (t_min > t_max)
					return false;
			}
			return true;
		}

		// 计算两个AABB相交区域的面积
		float IntersectionArea(const AABB& other)
		{
			// 先判断是否相交
			if (!Intersect(other))
				return 0.0f;

			// 计算相交区域的最小和最大坐标
			float interMinX = std::max(min.x, other.min.x);
			float interMaxX = std::min(max.x, other.max.x);
			float interMinY = std::max(min.y, other.min.y);
			float interMaxY = std::min(max.y, other.max.y);

			// 计算宽度和高度
			float width = interMaxX - interMinX;
			float height = interMaxY - interMinY;

			// 返回面积（确保不为负数）
			return std::max(0.0f, width) * std::max(0.0f, height);
		}

		inline float Area() const
		{
			return XRange() * YRange();
		}

		bool Contains(const glm::vec3& p)
		{
			return (p.x >= min.x && p.x <= max.x) &&
				(p.y >= min.y && p.y <= max.y);
		}

		bool Contains(const AABB& other)
		{
			return (other.min.x >= this->min.x && other.max.x <= this->max.x) &&
				(other.min.y >= this->min.y && other.max.y <= this->max.y);
		}

		float XRange() const { return max.x - min.x; }
		float YRange() const { return max.y - min.y; }
		float ZRange() const { return max.z - min.z; }
		float MaxRange() { return std::max(XRange(), YRange()); }
		float MinRange() { return std::min(XRange(), YRange()); }

		const glm::vec3& getMin() const { return min; }
		const glm::vec3& getMax() const { return max; }

		glm::vec3 Center()
		{
			return glm::vec3(
				(min.x + max.x) * 0.5f,
				(min.y + max.y) * 0.5f,
				(min.z + max.z) * 0.5f
			);
		}

		void Translate(const glm::vec3& translation)
		{
			min += translation;
			max += translation;
		}

		void Multiply(float scale)
		{
			glm::vec3 center = Center();
			float xRange = XRange() * scale;
			float yRange = YRange() * scale;
			min = center - glm::vec3(xRange / 2, yRange / 2, 0.0f);
			max = center + glm::vec3(xRange / 2, yRange / 2, 0.0f);
		}

	public:
		glm::vec3 min = glm::vec3(0);
		glm::vec3 max = glm::vec3(0);
	};
}
