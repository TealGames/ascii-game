#pragma once
#include "Utils/Data/WorldPosition.hpp"

struct LineData
{
	Vec2 m_StartPos;
	Vec2 m_EndPos;

	LineData(const WorldPosition3D& startPos, const WorldPosition3D& endPos);
};

using LineBuffer = std::vector<LineData>;
