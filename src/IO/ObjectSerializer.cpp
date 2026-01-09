#include "IO/ObjectSerializer.h"

template<typename T>
std::string serilize_to_string(T* p)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *p;

	return ss.str();
}

template<>
std::string serilize_to_string<Point2DGPU>(Point2DGPU* p)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *p;
	return ss.str();
}

template<>
std::string serilize_to_string<Line2DGPU>(Line2DGPU* line)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *line;
	return ss.str();
}

template<>
std::string serilize_to_string<Arc2DGPU>(Arc2DGPU* arc)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *arc;
	return ss.str();
}

template<>
std::string serilize_to_string<Circle2DGPU>(Circle2DGPU* circle)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *circle;
	return ss.str();
}

template<>
std::string serilize_to_string<Ellipse2DGPU>(Ellipse2DGPU* ellipse)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *ellipse;
	return ss.str();
}

template<>
std::string serilize_to_string<Polyline2DGPU>(Polyline2DGPU* polyline)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *polyline;
	return ss.str();
}

template<>
std::string serilize_to_string<Spline2DGPU>(Spline2DGPU* spline)
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);

	oa << *spline;
	return ss.str();
}

template<>
std::string serilize_to_string(EntityVGPU* ent)
{
	switch (ent->GetType())
	{
	case EntityType::Point:
		return serilize_to_string(dynamic_cast<Point2DGPU*>(ent));
	case EntityType::Line:
		return serilize_to_string(dynamic_cast<Line2DGPU*>(ent));
	case EntityType::Arc:
		return serilize_to_string(dynamic_cast<Arc2DGPU*>(ent));
	case EntityType::Circle:
		return serilize_to_string(dynamic_cast<Circle2DGPU*>(ent));
	case EntityType::Ellipse:
		return serilize_to_string(dynamic_cast<Ellipse2DGPU*>(ent));
	case EntityType::Polyline:
		return serilize_to_string(dynamic_cast<Polyline2DGPU*>(ent));
	case EntityType::Spline:
		return serilize_to_string(dynamic_cast<Spline2DGPU*>(ent));
	}
}

template<typename T>
T deserilize_from_string(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	T p;
	ia >> p;

	return p;
}

template<>
Point2DGPU deserilize_from_string<Point2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Point2DGPU p;
	ia >> p;

	return p;
}

template<>
Line2DGPU deserilize_from_string<Line2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Line2DGPU p;
	ia >> p;

	return p;
}

template<>
Circle2DGPU deserilize_from_string<Circle2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Circle2DGPU p;
	ia >> p;

	return p;
}

template<>
Arc2DGPU deserilize_from_string<Arc2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Arc2DGPU p;
	ia >> p;

	return p;
}

template<>
Polyline2DGPU deserilize_from_string<Polyline2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Polyline2DGPU p;
	ia >> p;

	return p;
}

template<>
Spline2DGPU deserilize_from_string<Spline2DGPU>(const std::string& data)
{
	std::stringstream ss(data);

	boost::archive::text_iarchive ia(ss);

	Spline2DGPU p;
	ia >> p;

	return p;
}