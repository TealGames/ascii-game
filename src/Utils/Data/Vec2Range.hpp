#pragma once
#include "Utils/Data/Vec2Type.hpp"

struct Vec2Range
{
	Vec2 m_Min;
	Vec2 m_Max;

	Vec2Range(const Vec2& minVec, const Vec2& maxVec);

	Vec2 GetRandom() const;
};

