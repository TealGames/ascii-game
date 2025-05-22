#include "pch.hpp"

#include "GUIRect.hpp"

GUIRect::GUIRect() : GUIRect(ScreenPosition(), Vec2Int()) {}

GUIRect::GUIRect(const ScreenPosition& topLeftPos, const Vec2Int& size) :
	//Note: as you go DOWN -> Y INCREASES
	m_topLeftPos(topLeftPos), m_bottomRightPos(m_topLeftPos.m_X + size.m_X, m_topLeftPos.m_Y+ size.m_Y) {}

ScreenPosition GUIRect::GetPosWithinRect(const NormalizedPosition& normalizedPos) const
{
	const Vec2Int size = GetSize();
	return {static_cast<int>(m_topLeftPos.m_X+ normalizedPos.GetPos().m_X * size.m_X),
			//We add the section from the top left y because Y POS INCREASES as you move DOWN
			static_cast<int>(m_topLeftPos.m_Y+ (1- normalizedPos.GetPos().m_Y)* size.m_Y)};
}

bool GUIRect::ContainsPos(const WorldPosition& pos) const
{
	//if (m_size.m_X == 0 || m_size.m_Y == 0) return false;

	return m_topLeftPos.m_X <= pos.m_X && pos.m_X <= m_bottomRightPos.m_X && 
			//Note: as y go down, y increases so top left has MIN Y and bottom right has MAX Y
		   m_topLeftPos.m_Y <= pos.m_Y && pos.m_Y <= m_bottomRightPos.m_Y;
}
bool GUIRect::ContainsPos(const ScreenPosition& pos) const
{
	return ContainsPos(WorldPosition(pos.m_X, pos.m_Y));
}

const ScreenPosition& GUIRect::GetTopleftPos() const
{
	return m_topLeftPos;
}
const ScreenPosition& GUIRect::GetBottomRightPos() const
{
	return m_bottomRightPos;
}
Vec2Int GUIRect::GetSize() const
{
	//Note: bottom right pos should have greater Y since y increases as you go down
	return Vec2Int(m_bottomRightPos.m_X - m_topLeftPos.m_X, m_bottomRightPos.m_Y- m_topLeftPos.m_Y);
}

void GUIRect::SetTopLeftPos(const ScreenPosition& pos)
{
	m_topLeftPos = pos;
}
void GUIRect::SetSize(const Vec2Int& size)
{
	m_bottomRightPos = m_topLeftPos + Vec2Int(size.m_X, -size.m_Y);
}

std::string GUIRect::ToString() const
{
	return std::format("[TopLeft: {} BottomRight: {} Size: {}]", 
		m_topLeftPos.ToString(), m_bottomRightPos.ToString(), GetSize().ToString());
}