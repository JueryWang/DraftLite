#pragma once
#include <string>
#include "Graphics/Primitives.h"
#include "glm/glm.hpp"
#include "Common/ProgressInfo.h"
#include "Path/Path.h"

std::string GenGodeByPath(Path2D* path, SimulateStatus* Mstatus, const glm::mat4& baseMat);