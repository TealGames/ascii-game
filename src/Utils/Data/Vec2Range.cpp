#include "pch.hpp"
#include "Vec2Range.hpp"
#include "Utils/HelperFunctions.hpp"

Vec2Range::Vec2Range(const Vec2& minVec, const Vec2& maxVec)
	: m_Min(minVec), m_Max(maxVec) {}

Vec2 Vec2Range::GetRandom() const
{
	return Vec2(Utils::GenerateRandomFloat(m_Min.m_X, m_Max.m_X), 
				Utils::GenerateRandomFloat(m_Min.m_Y, m_Max.m_Y));
}
