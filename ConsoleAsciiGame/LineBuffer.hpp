#pragma once
#include "Point2D.hpp"
#include "WorldPosition.hpp"

struct LineData
{
	Utils::Point2D m_StartPos;
	Utils::Point2D m_EndPos;

	LineData(const WorldPosition& startPos, const WorldPosition& endPos);
};

using LineBuffer = std::vector<LineData>;
