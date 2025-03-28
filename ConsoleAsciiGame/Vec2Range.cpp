#include "pch.hpp"
#include "Vec2Range.hpp"

Vec2Range::Vec2Range(const Vec2& minVec, const Vec2& maxVec)
	: m_Min(minVec), m_Max(maxVec) {}

Vec2 Vec2Range::GetRnadom() const
{
	return GenerateRandomVec2(m_Min, m_Max);
}
