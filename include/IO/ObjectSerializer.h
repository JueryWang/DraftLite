#pragma once
#include <glm/glm.hpp>
#include <sstream>
#include <vector>
#include <string>
#include <boost/serialization/split_free.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <Graphics/DrawEntity.h>
#include <Graphics/Primitives.h>

using namespace CNCSYS;

namespace boost {
	namespace serialization {
		template<class Archive>
		void save(Archive& ar, const glm::vec3& v, const unsigned int version) {
			ar& v.x;  // 序列化 x 分量
			ar& v.y;  // 序列化 y 分量
			ar& v.z;  // 序列化 z 分量
		}
		template<class Archive>
		void load(Archive& ar, glm::vec3& v, const unsigned int version) {
			ar& v.x;  // 反序列化 x 分量
			ar& v.y;  // 反序列化 y 分量
			ar& v.z;  // 反序列化 z 分量
		}

		template<class Archive>
		void save(Archive& ar, const glm::vec4& v, const unsigned int version) {
			ar& v.x;  // 序列化 x 分量
			ar& v.y;  // 序列化 y 分量
			ar& v.z;  // 序列化 z 分量
			ar& v.w;  // 序列化 w 分量
		}
		template<class Archive>
		void load(Archive& ar, glm::vec4& v, const unsigned int version) {
			ar& v.x;  // 反序列化 x 分量
			ar& v.y;  // 反序列化 y 分量
			ar& v.z;  // 反序列化 z 分量
			ar& v.w;  // 反序列化 w 分量
		}


		// 绑定 save 和 load 函数到 glm::vec3 类型
		template<class Archive>
		void serialize(Archive& ar, glm::vec3& v, const unsigned int version) {
			split_free(ar, v, version);
		}

		template<class Archive>
		void serialize(Archive& ar, glm::vec4& v, const unsigned int version) {
			split_free(ar, v, version);
		}

		template<class Archive>
		void save(Archive& ar, const glm::mat4& mat, const unsigned int version) {
			for (int col = 0; col < 4; ++col)
			{
				for (int row = 0; row < 4; ++row){
					ar& mat[col][row];
				}
			}
		}

		template<class Archive>
		void load(Archive& ar, glm::mat4& mat, const unsigned int version) {
			for (int col = 0; col < 4; ++col)
			{
				for (int row = 0; row < 4; ++row) {
					ar& mat[col][row];
				}
			}
		}

		template<class Archive>
		void serialize(Archive& ar, glm::mat4& mat, const unsigned int version) {
			split_free(ar, mat, version);  // 使用 split_free 分离序列化和反序列化
		}
	} // namespace serialization
} // namespace boost


template<typename T>
std::string serilize_to_string(T* p);

template<>
std::string serilize_to_string<Point2DGPU>(Point2DGPU* p);

template<>
std::string serilize_to_string<Line2DGPU>(Line2DGPU* line);

template<>
std::string serilize_to_string<Arc2DGPU>(Arc2DGPU* arc);

template<>
std::string serilize_to_string<Circle2DGPU>(Circle2DGPU* circle);

template<>
std::string serilize_to_string<Ellipse2DGPU>(Ellipse2DGPU* ellipse);

template<>
std::string serilize_to_string<Polyline2DGPU>(Polyline2DGPU* polyline);

template<>
std::string serilize_to_string<Spline2DGPU>(Spline2DGPU* spline);

template<>
std::string serilize_to_string(EntityVGPU* ent);

template<typename T>
T deserilize_from_string(const std::string& data);

template<>
Point2DGPU deserilize_from_string<Point2DGPU>(const std::string& data);

template<>
Line2DGPU deserilize_from_string<Line2DGPU>(const std::string& data);

template<>
Circle2DGPU deserilize_from_string<Circle2DGPU>(const std::string& data);

template<>
Arc2DGPU deserilize_from_string<Arc2DGPU>(const std::string& data);

template<>
Polyline2DGPU deserilize_from_string<Polyline2DGPU>(const std::string& data);

template<>
Spline2DGPU deserilize_from_string<Spline2DGPU>(const std::string& data);