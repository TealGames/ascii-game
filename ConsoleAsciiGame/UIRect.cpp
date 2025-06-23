#include "pch.hpp"

#include "UIRect.hpp"

UIRect::UIRect() : UIRect(ScreenPosition(), Vec2Int()) {}

UIRect::UIRect(const ScreenPosition& topLeftPos, const Vec2Int& size) :
	//Note: as you go DOWN -> Y INCREASES
	m_TopLeftPos(topLeftPos), m_BottomRightPos(m_TopLeftPos.m_X + size.m_X, m_TopLeftPos.m_Y+ size.m_Y) {}

ScreenPosition UIRect::GetPosWithinRect(const NormalizedPosition& normalizedPos) const
{
	const Vec2Int size = GetSize();
	return {static_cast<int>(m_TopLeftPos.m_X+ normalizedPos.GetPos().m_X * size.m_X),
			//We add the section from the top left y because Y POS INCREASES as you move DOWN
			static_cast<int>(m_TopLeftPos.m_Y+ (1- normalizedPos.GetPos().m_Y)* size.m_Y)};
}

bool UIRect::ContainsPos(const WorldPosition& pos) const
{
	//if (m_size.m_X == 0 || m_size.m_Y == 0) return false;

	return m_TopLeftPos.m_X <= pos.m_X && pos.m_X <= m_BottomRightPos.m_X && 
			//Note: as y go down, y increases so top left has MIN Y and bottom right has MAX Y
		   m_TopLeftPos.m_Y <= pos.m_Y && pos.m_Y <= m_BottomRightPos.m_Y;
}
bool UIRect::ContainsPos(const ScreenPosition& pos) const
{
	return ContainsPos(WorldPosition(pos.m_X, pos.m_Y));
}

Vec2Int UIRect::GetSize() const
{
	//Note: bottom right pos should have greater Y since y increases as you go down
	return Vec2Int(m_BottomRightPos.m_X - m_TopLeftPos.m_X, m_BottomRightPos.m_Y- m_TopLeftPos.m_Y);
}

void UIRect::SetTopLeftPos(const ScreenPosition& pos)
{
	m_TopLeftPos = pos;
}
void UIRect::SetSize(const Vec2Int& size)
{
	m_BottomRightPos = m_TopLeftPos + Vec2Int(size.m_X, -size.m_Y);
}

std::string UIRect::ToString() const
{
	return std::format("[TopLeft: {} BottomRight: {} Size: {}]", 
		m_TopLeftPos.ToString(), m_BottomRightPos.ToString(), GetSize().ToString());
}