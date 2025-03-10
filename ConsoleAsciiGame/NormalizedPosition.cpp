#include "pch.hpp"
#include "NormalizedPosition.hpp"
#include "Debug.hpp"

NormalizedPosition::NormalizedPosition() : 
	NormalizedPosition(0, 0) {}

NormalizedPosition::NormalizedPosition(const Vec2& pos) : 
	NormalizedPosition(pos.m_X, pos.m_Y) {}

bool NormalizedPosition::IsValidPos() const
{
	return 0 <= m_pos.m_X && m_pos.m_X <= 1 
		&& 0 <= m_pos.m_Y && m_pos.m_Y <= 1;
}

NormalizedPosition::NormalizedPosition(const float& x, const float& y) : m_pos()
{
	SetPos({ x, y });
}

const Vec2& NormalizedPosition::GetPos() const
{
	if (!Assert(this, IsValidPos(), std::format("Tried to retrieve normalized pos IMMUTABLE, "
		"but it is an invalid state: {}", m_pos.ToString())))
		throw std::invalid_argument("Invalid normalized pos state");

	return m_pos;
}
Vec2& NormalizedPosition::GetPosMutable()
{
	if (!Assert(this, IsValidPos(), std::format("Tried to retrieve normalized pos MUTABLE, "
		"but it is an invalid state: {}", m_pos.ToString())))
		throw std::invalid_argument("Invalid normalized pos state");

	return m_pos;
}

void NormalizedPosition::SetPos(const Vec2& relativePos)
{
	m_pos.m_X = std::clamp(relativePos.m_X, float(0), float(1));
	m_pos.m_Y = std::clamp(relativePos.m_Y, float(0), float(1));
}