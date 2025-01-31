#pragma once
#include <vector>
#include "ShapeType.hpp"
#include "ScreenPosition.hpp"
#include "Point2D.hpp"
#include "raylib.h"

struct RectangleOutlineData
{
	//Size in [WIDTH, HEIGHT]
	Utils::Point2D m_Size;
	ScreenPosition m_Position;

	RectangleOutlineData(const Utils::Point2D& size, const ScreenPosition& pos);
};

struct ColliderOutlineBuffer
{
	std::vector<RectangleOutlineData> m_RectangleBuffer;

	ColliderOutlineBuffer();

	void ClearAll();
	void AddRectangle(const RectangleOutlineData& rectangleData);

	bool HasData() const;
};

