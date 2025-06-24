#pragma once
#include <vector>
#include "ScreenPosition.hpp"
#include "Vec2.hpp"

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

