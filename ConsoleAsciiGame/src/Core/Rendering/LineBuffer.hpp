#pragma once
#include "Utils/Data/WorldPosition.hpp"

struct LineData
{
	Vec2 m_StartPos;
	Vec2 m_EndPos;

	LineData(const WorldPosition& startPos, const WorldPosition& endPos);
};

using LineBuffer = std::vector<LineData>;
