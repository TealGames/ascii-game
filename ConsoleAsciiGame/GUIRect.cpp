#include "pch.hpp"

#include "GUIRect.hpp"
#include "Point2DInt.hpp"

GUIRect::GUIRect() : GUIRect(ScreenPosition{}, Utils::Point2DInt{}) {}

GUIRect::GUIRect(const ScreenPosition& topLeftPos, const Utils::Point2DInt& size) :
	m_topLeftPos(topLeftPos), m_size(size), m_bottomRightPos(GetPosWithinRect({1, 0})) {}

ScreenPosition GUIRect::GetPosWithinRect(const NormalizedPosition& normalizedPos) const
{
	return {static_cast<int>(m_topLeftPos.m_X+ normalizedPos.GetPos().m_X * m_size.m_X), 
			//We add the section from the top left y because Y POS INCREASES as you move DOWN
			static_cast<int>(m_topLeftPos.m_Y+ (1- normalizedPos.GetPos().m_Y)* m_size.m_Y)};
}

bool GUIRect::ContainsPos(const ScreenPosition& pos) const
{
	if (m_size.m_X == 0 || m_size.m_Y == 0) return false;

	return m_topLeftPos.m_X <= pos.m_X && pos.m_X <= m_bottomRightPos.m_X && 
			//Note: as y go down, y increases so top left has MIN Y and bottom right has MAX Y
		   m_topLeftPos.m_Y <= pos.m_Y <= m_bottomRightPos.m_Y;
}

const ScreenPosition& GUIRect::GetTopleftPos() const
{
	return m_topLeftPos;
}
const ScreenPosition& GUIRect::GetBottomRightPos() const
{
	return m_bottomRightPos;
}
const Utils::Point2DInt& GUIRect::GetSize() const
{
	return m_size;
}

void GUIRect::SetTopLeftPos(const ScreenPosition& pos)
{
	m_topLeftPos = pos;
	m_bottomRightPos = GetPosWithinRect({ 1, 0 });
}
void GUIRect::SetSize(const Utils::Point2DInt& size)
{
	m_size = size;
	m_bottomRightPos = GetPosWithinRect({ 1, 0 });
}

std::string GUIRect::ToString() const
{
	return std::format("[TopLeft: {} BottomRight: {} Size: {}]", 
		m_topLeftPos.ToString(), m_bottomRightPos.ToString(), m_size.ToString());
}