#include "Common/HistoryCmds.h"
#include "IO/ObjectSerializer.h"
#include "Graphics/Sketch.h"

using namespace CNCSYS;

HistoryRecorder* HistoryRecorder::instance = nullptr;

HistoryRecorder* HistoryRecorder::GetInstance()
{
	if (HistoryRecorder::instance == nullptr)
	{
		HistoryRecorder::instance = new HistoryRecorder(50);
	}
	return HistoryRecorder::instance;
}

void HistoryRecorder::SetSketch(SketchGPU* sketch)
{
	sketchHandle = sketch;
}

void HistoryRecorder::PushRecord(const HistoryRecord& command)
{
	if (revokeCmds.size() > capacity)
		revokeCmds.pop_back();
	revokeCmds.push_front(command);
}
void HistoryRecorder::Revoke()
{
	if (revokeCmds.size() > 0)
	{
		std::vector<OperationCommand> restoreCmds;
		//撤销队首出栈
		HistoryRecord& record = revokeCmds.front();
		for (auto& cmd : record.commands)
		{
			if (cmd.first != nullptr)
			{
				//恢复命令将当前状态入栈
				switch (cmd.first->GetType())
				{
				case EntityType::Point:
				{
					Point2DGPU* current_point = dynamic_cast<Point2DGPU*>(cmd.first);
					if (cmd.second.size())
					{
						Point2DGPU revoked_point = deserilize_from_string<Point2DGPU>(cmd.second);
						current_point->Copy(&revoked_point);
						sketchHandle->UpdateEntityBox(current_point, current_point->bbox);
					}
					else
					{
						current_point->isVisible = false;
					}
					std::string current_serilized = serilize_to_string<Point2DGPU>(current_point);
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				case EntityType::Line:
				{
					Line2DGPU* current_line = dynamic_cast<Line2DGPU*>(cmd.first);
					if (cmd.second.size())
					{
						Line2DGPU revoked_line = deserilize_from_string<Line2DGPU>(cmd.second);
						current_line->Copy(&revoked_line);
						sketchHandle->UpdateEntityBox(current_line, current_line->bbox);
					}
					else
					{
						current_line->isVisible = false;
					}
					std::string current_serilized = serilize_to_string<Line2DGPU>(current_line);
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				case EntityType::Circle:
				{
					Circle2DGPU* current_circle = dynamic_cast<Circle2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Circle2DGPU>(current_circle);
					if (cmd.second.size())
					{
						Circle2DGPU revoked_circle = deserilize_from_string<Circle2DGPU>(cmd.second);
						current_circle->Copy(&revoked_circle);
						sketchHandle->UpdateEntityBox(current_circle, current_circle->bbox);
					}
					else
					{
						current_circle->isVisible = false;
					}
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				case EntityType::Arc:
				{
					Arc2DGPU* current_arc = dynamic_cast<Arc2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Arc2DGPU>(current_arc);
					if (cmd.second.size())
					{
						Arc2DGPU revoked_arc = deserilize_from_string<Arc2DGPU>(cmd.second);
						current_arc->Copy(&revoked_arc);
						sketchHandle->UpdateEntityBox(current_arc, current_arc->bbox);
					}
					else
					{
						current_arc->isVisible = false;
					}
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				case EntityType::Polyline:
				{
					Polyline2DGPU* current_poly = dynamic_cast<Polyline2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Polyline2DGPU>(current_poly);
					if (cmd.second.size())
					{
						Polyline2DGPU revoked_poly = deserilize_from_string<Polyline2DGPU>(cmd.second);
						current_poly->Copy(&revoked_poly);
						sketchHandle->UpdateEntityBox(current_poly, current_poly->bbox);
					}
					else
					{
						current_poly->isVisible = false;
					}
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				case EntityType::Spline:
				{
					Spline2DGPU* current_spline = dynamic_cast<Spline2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Spline2DGPU>(current_spline);
					if (cmd.second.size())
					{
						Spline2DGPU revoked_spline = deserilize_from_string<Spline2DGPU>(cmd.second);
						current_spline->Copy(&revoked_spline);
						sketchHandle->UpdateEntityBox(current_spline, current_spline->bbox);
					}
					else
					{
						current_spline->isVisible = false;
					}
					OperationCommand restoreCmd = std::make_pair(cmd.first, current_serilized);
					restoreCmds.push_back(restoreCmd);
					break;
				}
				}
			}
			else
			{
				try
				{
					Point2DGPU revoked_point = deserilize_from_string<Point2DGPU>(cmd.second);
					Point2DGPU* new_point = new Point2DGPU();
					new_point->Copy(&revoked_point);
					sketchHandle->AddEntity(new_point);
					continue;
				}
				catch (...) {}

				try
				{
					Line2DGPU revoked_line = deserilize_from_string<Line2DGPU>(cmd.second);
					Line2DGPU* new_line = new Line2DGPU();
 					new_line->Copy(&revoked_line);
					sketchHandle->AddEntity(new_line);
					auto entGroups = sketchHandle->GetEntityGroups();
					bool ifBreak = false;
					for (EntGroup* group : entGroups)
					{
						for (EntRingConnection* ring : group->rings)
						{
							if (ring->ringId == new_line->ringId)
							{
								ring->conponents;
							}
						}
					}
					continue;
				}
				catch (...) {}

				try
				{
					Circle2DGPU revoked_circle = deserilize_from_string<Circle2DGPU>(cmd.second);
					Circle2DGPU* new_circle = new Circle2DGPU();
					new_circle->Copy(&revoked_circle);
					sketchHandle->AddEntity(new_circle);
					continue;
				}
				catch (...) {}

				try 
				{
					Polyline2DGPU revoked_polyline = deserilize_from_string<Polyline2DGPU>(cmd.second);
					Polyline2DGPU* new_polyline = new Polyline2DGPU();
					new_polyline->Copy(&revoked_polyline);
					sketchHandle->AddEntity(new_polyline);
					continue;
				}
				catch (...) {}

				try 
				{
					Arc2DGPU revoked_arc = deserilize_from_string<Arc2DGPU>(cmd.second);
					Arc2DGPU* new_arc = new Arc2DGPU();
					new_arc->Copy(&revoked_arc);
					sketchHandle->AddEntity(new_arc);
					continue;
				}
				catch (...) {}

				try
				{
					Spline2DGPU revoked_spline = deserilize_from_string<Spline2DGPU>(cmd.second);
					Spline2DGPU* new_spline = new Spline2DGPU();
					new_spline->Copy(&revoked_spline);
					sketchHandle->AddEntity(new_spline);
					continue;
				}
				catch (...) {}
			}
		}
		if (record.cleanFunc)
		{
			record.cleanFunc(this);
		}
		HistoryRecord restoreRec;
		restoreRec.commands = restoreCmds;
		restoreRec.cleanFunc = std::move(record.cleanFunc);
		if (recoverCmds.size() > capacity)
			recoverCmds.pop_back();
		recoverCmds.push_front(restoreRec);
		revokeCmds.pop_front();
	}
}
void HistoryRecorder::Restore()
{
	if (!recoverCmds.empty())
	{
		std::vector<OperationCommand> _revokeCmds;

		HistoryRecord& record = recoverCmds.front();
		for (auto& cmd : record.commands)
		{
			if (cmd.first != nullptr)
			{
				switch (cmd.first->GetType())
				{
				case EntityType::Point:
				{
					Point2DGPU* current_point = dynamic_cast<Point2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Point2DGPU>(current_point);
					if (cmd.second.size())
					{
						Point2DGPU recovered_point = deserilize_from_string<Point2DGPU>(cmd.second);
						current_point->Copy(&recovered_point);
						sketchHandle->UpdateEntityBox(current_point,current_point->bbox);
					}
					OperationCommand revokeCmd = std::make_pair(current_point, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				case EntityType::Line:
				{
					Line2DGPU* current_line = dynamic_cast<Line2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Line2DGPU>(current_line);
					if (cmd.second.size())
					{
						Line2DGPU recovered_line = deserilize_from_string<Line2DGPU>(cmd.second);
						current_line->Copy(&recovered_line);
						sketchHandle->UpdateEntityBox(current_line,current_line->bbox);
					}
					OperationCommand revokeCmd = std::make_pair(current_line, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				case EntityType::Arc:
				{
					Arc2DGPU* current_arc = dynamic_cast<Arc2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Arc2DGPU>(current_arc);
					if (cmd.second.size())
					{
						Arc2DGPU recovered_arc = deserilize_from_string<Arc2DGPU>(cmd.second);
						current_arc->Copy(&recovered_arc);
						sketchHandle->UpdateEntityBox(current_arc,current_arc->bbox);
					}
					OperationCommand revokeCmd = std::make_pair(current_arc, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				case EntityType::Circle:
				{
					Circle2DGPU* current_circle = dynamic_cast<Circle2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Circle2DGPU>(current_circle);
					if (cmd.second.size())
					{
						Circle2DGPU recovered_circle = deserilize_from_string<Circle2DGPU>(cmd.second);
						current_circle->Copy(&recovered_circle);
						sketchHandle->UpdateEntityBox(current_circle, current_circle->bbox);
					}
					OperationCommand revokeCmd = std::make_pair(current_circle, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				case EntityType::Polyline:
				{
					Polyline2DGPU* current_poly = dynamic_cast<Polyline2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Polyline2DGPU>(current_poly);
					if (cmd.second.size())
					{
						Polyline2DGPU recovered_poly = deserilize_from_string<Polyline2DGPU>(cmd.second);
						current_poly->Copy(&recovered_poly);
						sketchHandle->UpdateEntityBox(current_poly,current_poly->bbox);
					}

					OperationCommand revokeCmd = std::make_pair(current_poly, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				case EntityType::Spline:
				{
					Spline2DGPU* current_spline = dynamic_cast<Spline2DGPU*>(cmd.first);
					std::string current_serilized = serilize_to_string<Spline2DGPU>(current_spline);
					if (cmd.second.size())
					{
						Spline2DGPU recovered_spline = deserilize_from_string<Spline2DGPU>(cmd.second);
						current_spline->Copy(&recovered_spline);
						sketchHandle->UpdateEntityBox(current_spline, current_spline->bbox);
					}
					OperationCommand revokeCmd = std::make_pair(current_spline, current_serilized);
					_revokeCmds.push_back(revokeCmd);
					break;
				}
				default:
					break;
				}
			}
		}

		HistoryRecord revokeRec;
		revokeRec.commands = _revokeCmds;
		revokeRec.cleanFunc = std::move(record.cleanFunc);
		revokeCmds.push_front(revokeRec);
		recoverCmds.pop_front();
	}
}

HistoryRecorder::HistoryRecorder(int maxHistory) : capacity(maxHistory)
{

}
HistoryRecorder::~HistoryRecorder()
{

}