#pragma once
#include <vector>
#include "Utils/Math/ScreenPosition.hpp"
#include "Utils/Math/Vec2Type.hpp"

struct RectangleOutlineData
{
	//Size in [WIDTH, HEIGHT]
	Vec2 m_Size;
	ScreenPosition m_Position;

	RectangleOutlineData(const Vec2& size, const ScreenPosition& pos);
};

struct ColliderOutlineBuffer
{
	std::vector<RectangleOutlineData> m_RectangleBuffer;

	ColliderOutlineBuffer();

	void ClearAll();
	void AddRectangle(const RectangleOutlineData& rectangleData);

	bool HasData() const;
};

