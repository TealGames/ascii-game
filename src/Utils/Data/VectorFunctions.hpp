#pragma once
#include "Utils/Data/Vec2Type.hpp"

namespace Vec
{
	float GetDistance(const Vec2& vec1, const Vec2& vec2);
	float GetDistance(const Vec2& vec1, const Vec2& vec2);

	Vec2 GenerateRandomVec2(const Vec2& minVec, const Vec2 maxVec);
	Vec2 GenerateRandomDir();

	bool IsWithinBounds(const Vec2& targetPos, const Vec2& minPos, const Vec2& maxPos);
}