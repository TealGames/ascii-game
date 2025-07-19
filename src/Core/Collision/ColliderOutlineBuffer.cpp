#include "pch.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"

RectangleOutlineData::RectangleOutlineData(const Vec2& size, const ScreenPosition& pos) :
	m_Size(size), m_Position(pos) {}

ColliderOutlineBuffer::ColliderOutlineBuffer() : m_RectangleBuffer() {}

void ColliderOutlineBuffer::ClearAll()
{
	m_RectangleBuffer.clear();
}

void ColliderOutlineBuffer::AddRectangle(const RectangleOutlineData& rectangleData)
{
	m_RectangleBuffer.emplace_back(rectangleData);
}

bool ColliderOutlineBuffer::HasData() const
{
	return !m_RectangleBuffer.empty();
}
