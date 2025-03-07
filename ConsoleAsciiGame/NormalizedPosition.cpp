#include "pch.hpp"
#include "NormalizedPosition.hpp"

NormalizedPosition::NormalizedPosition() : 
	NormalizedPosition(0, 0) {}

NormalizedPosition::NormalizedPosition(const Vec2& pos) : 
	NormalizedPosition(pos.m_X, pos.m_Y) {}

NormalizedPosition::NormalizedPosition(const float& x, const float& y) : m_pos()
{
	SetPos({ x, y });
}

const Vec2& NormalizedPosition::GetPos() const
{
	return m_pos;
}

void NormalizedPosition::SetPos(const Vec2& relativePos)
{
	m_pos.m_X = std::clamp(relativePos.m_X, float(0), float(1));
	m_pos.m_Y = std::clamp(relativePos.m_Y, float(0), float(1));
}