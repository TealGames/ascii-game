#include "Utils/Data/VectorFunctions.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Vec
{
	Vec2 GenerateRandomVec2(const Vec2& minVec, const Vec2 maxVec)
	{
		if (minVec.m_X > maxVec.m_X) return {};
		if (minVec.m_Y > maxVec.m_Y) return {};

		return Vec2{ static_cast<float>(Utils::GenerateRandomDouble(minVec.m_X, maxVec.m_X)),
					 static_cast<float>(Utils::GenerateRandomDouble(minVec.m_Y, maxVec.m_Y)) };
	}
	Vec2 GenerateRandomDir()
	{
		return GenerateRandomVec2(Vec2{ -1, -1 }, Vec2{ 1, 1 });
	}

	bool IsWithinBounds(const Vec2& targetPos, const Vec2& minPos, const Vec2& maxPos)
	{
		return minPos.m_X <= targetPos.m_X && targetPos.m_X <= maxPos.m_X &&
			minPos.m_Y <= targetPos.m_Y && targetPos.m_Y <= maxPos.m_Y;
	}
}