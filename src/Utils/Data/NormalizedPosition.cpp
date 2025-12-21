#include "pch.hpp"
#include "Utils/Data/NormalizedPosition.hpp"
#include "Utils/Debug.hpp"

const Vec2 NormalizedPosition::TOP_LEFT = { MIN, MAX };
const Vec2 NormalizedPosition::TOP_RIGHT = { MAX, MAX };
const Vec2 NormalizedPosition::BOTTOM_LEFT = { MIN, MIN };
const Vec2 NormalizedPosition::BOTTOM_RIGHT = { MAX, MIN };
const Vec2 NormalizedPosition::CENTER = { HALF, HALF };
const Vec2 NormalizedPosition::BOTTOM_CENTER = { HALF, MIN };
const Vec2 NormalizedPosition::TOP_CENTER = { HALF, MAX };

NormalizedPosition::NormalizedPosition() : 
	NormalizedPosition(0, 0) {}

NormalizedPosition::NormalizedPosition(const Vec2& pos) : 
	NormalizedPosition(pos.m_X, pos.m_Y) {}

NormalizedPosition::NormalizedPosition(const float& x, const float& y) : m_X(x), m_Y(y) {}
NormalizedPosition::NormalizedPosition(const NormalizedValue x, const NormalizedValue y) 
	: m_X(x), m_Y(y) {}

NormalizedPosition::NormalizedPosition(const float x, const NormalizedValue y) : NormalizedPosition(NormalizedValue(x), y) {}
NormalizedPosition::NormalizedPosition(const NormalizedValue x, const float y) : NormalizedPosition(x, NormalizedValue(y)) {}

const Vec2 NormalizedPosition::GetPos() const
{
	return Vec2(m_X.GetValue(), m_Y.GetValue());
}

const float NormalizedPosition::GetX() const { return m_X.GetValue(); }
const float NormalizedPosition::GetY() const { return m_Y.GetValue(); }

void NormalizedPosition::SetPos(const Vec2& relativePos)
{
	/*SetPosX(relativePos.m_X);
	SetPosY(relativePos.m_Y);*/
	m_X = relativePos.m_X;
	m_Y = relativePos.m_Y;
}

bool NormalizedPosition::IsZero() const 
{ 
	return m_X == 0 && m_Y == 0;
}

std::string NormalizedPosition::ToString() const
{
	return std::format("({},{})", m_X.ToString(), m_Y.ToString());
}

NormalizedPosition NormalizedPosition::operator+(const NormalizedPosition& other) const
{
	return NormalizedPosition(m_X + other.m_X, m_Y + other.m_Y);
}
NormalizedPosition NormalizedPosition::operator-(const NormalizedPosition& other) const
{
	return NormalizedPosition(m_X - other.m_X, m_Y - other.m_Y);
}
NormalizedPosition NormalizedPosition::operator*(const NormalizedPosition& other) const
{
	return NormalizedPosition(m_X * other.m_X, m_Y * other.m_Y);
}
NormalizedPosition NormalizedPosition::operator*(const float& scalar) const
{
	return NormalizedPosition(m_X * scalar, m_Y * scalar);
}
NormalizedPosition NormalizedPosition::operator/(const float& scalar) const
{
	ENGINE_ASSERT(scalar != 0, "Tried to divide a normalized position: {} by a 0 value scalar", ToString());
	return NormalizedPosition(m_X / scalar, m_Y / scalar);
}

bool NormalizedPosition::operator==(const NormalizedPosition& other) const
{
	return m_X == other.m_X && m_Y == other.m_Y;
}
bool NormalizedPosition::operator!=(const NormalizedPosition& other) const
{
	return !(*this == other);
}
bool NormalizedPosition::operator>(const NormalizedPosition& other) const
{
	return m_X > other.m_X && m_Y > other.m_Y;
}
bool NormalizedPosition::operator>=(const NormalizedPosition& other) const
{
	return *this > other || *this == other;
}
bool NormalizedPosition::operator<(const NormalizedPosition& other) const
{
	return m_X < other.m_X && m_Y < other.m_Y;
}
bool NormalizedPosition::operator<=(const NormalizedPosition& other) const
{
	return *this < other || *this == other;
}

NormalizedPosition GetSizeFromCorners(const NormalizedPosition& topLeft, const NormalizedPosition& bottomRight)
{
	return NormalizedPosition(bottomRight.GetX() - topLeft.GetX(), topLeft.GetY() - bottomRight.GetY());
}
NormalizedPosition GetBottomRight(const NormalizedPosition& topLeft, const NormalizedPosition& size)
{
	return NormalizedPosition(topLeft.GetX() + size.GetX(), topLeft.GetY() - size.GetY());
}
NormalizedPosition GetTopLeft(const NormalizedPosition& bottomRight, const NormalizedPosition& size)
{
	return NormalizedPosition(bottomRight.GetX() - size.GetX(), bottomRight.GetY() + size.GetY());
}