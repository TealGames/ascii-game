#include "pch.hpp"
#include "RelativeGUIRect.hpp"

RelativeGUIRect::RelativeGUIRect() : RelativeGUIRect({}, {}) {}
RelativeGUIRect::RelativeGUIRect(const NormalizedPosition& topLeft, const NormalizedPosition& size) :
	m_topLeftPos(topLeft), m_bottomRightPos(GetBottomRight(topLeft, size)) {}

ScreenPosition GetRealPos(const NormalizedPosition relativePos, const ScreenPosition& parentSize)
{
	return ScreenPosition(parentSize.m_X * relativePos.GetX(), parentSize.m_Y * relativePos.GetY());
}

void RelativeGUIRect::SetSize(const NormalizedPosition& size, const bool keepTopLeft)
{
	if (keepTopLeft) m_bottomRightPos = GetBottomRight(m_topLeftPos, size);
	else m_topLeftPos = GetTopLeft(m_bottomRightPos, size);
}
void RelativeGUIRect::SetMaxSize()
{
	m_bottomRightPos = NormalizedPosition::BOTTOM_RIGHT;
	m_topLeftPos = NormalizedPosition::TOP_LEFT;
}
void RelativeGUIRect::SetTopLeft(const NormalizedPosition& topLeft)
{
	m_topLeftPos = topLeft;
}
void RelativeGUIRect::SetBottomRight(const NormalizedPosition& bottomRight)
{
	m_bottomRightPos = bottomRight;
}
void RelativeGUIRect::SetBounds(const NormalizedPosition& topLeft, const NormalizedPosition& bottomRight)
{
	m_topLeftPos = topLeft;
	m_bottomRightPos = bottomRight;
}

NormalizedPosition RelativeGUIRect::GetSize() const
{
	return GetSizeFromCorners(m_topLeftPos, m_bottomRightPos);
}
ScreenPosition RelativeGUIRect::GetSize(const ScreenPosition parentSize) const
{
	return GetRealPos(GetSize(), parentSize);
}

const NormalizedPosition& RelativeGUIRect::GetTopLeftPos() const
{
	return m_topLeftPos;
}
ScreenPosition RelativeGUIRect::GetTopLeftPos(const ScreenPosition parentSize) const
{
	return GetRealPos(m_topLeftPos, parentSize);
}

const NormalizedPosition& RelativeGUIRect::GetBottomRighttPos() const
{
	return m_bottomRightPos;
}
ScreenPosition RelativeGUIRect::GetBottomRighttPos(const ScreenPosition parentSize) const
{
	return GetRealPos(m_bottomRightPos, parentSize);
}

std::string RelativeGUIRect::ToString() const
{
	return std::format("[]");
}
