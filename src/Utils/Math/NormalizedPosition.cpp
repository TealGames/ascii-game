#include "pch.hpp"
#include "Utils/Math/NormalizedPosition.hpp"
#include "Utils/Debug.hpp"

NormalizedPos::NormalizedPos() : 
	NormalizedPos(0, 0) {}

NormalizedPos::NormalizedPos(const Vec2& pos) : 
	NormalizedPos(pos.m_X, pos.m_Y) {}

NormalizedPos::NormalizedPos(const float& x, const float& y) : m_X(x), m_Y(y) {}
NormalizedPos::NormalizedPos(const NormalizedValue x, const NormalizedValue y) 
	: m_X(x), m_Y(y) {}

NormalizedPos::NormalizedPos(const float x, const NormalizedValue y) : NormalizedPos(NormalizedValue(x), y) {}
NormalizedPos::NormalizedPos(const NormalizedValue x, const float y) : NormalizedPos(x, NormalizedValue(y)) {}

Vec2 NormalizedPos::AsVec2() const
{
	return Vec2(m_X.GetValue(), m_Y.GetValue());
}
float NormalizedPos::GetX() const { return m_X.GetValue(); }
float NormalizedPos::GetY() const { return m_Y.GetValue(); }

void NormalizedPos::SetPos(const Vec2& relativePos)
{
	/*SetPosX(relativePos.m_X);
	SetPosY(relativePos.m_Y);*/
	m_X = relativePos.m_X;
	m_Y = relativePos.m_Y;
}

bool NormalizedPos::IsZero() const 
{ 
	return m_X == 0 && m_Y == 0;
}

std::string NormalizedPos::ToString() const
{
	return std::format("({},{})", m_X.ToString(), m_Y.ToString());
}

NormalizedPos NormalizedPos::operator+(const NormalizedPos& other) const
{
	return NormalizedPos(m_X + other.m_X, m_Y + other.m_Y);
}
NormalizedPos NormalizedPos::operator-(const NormalizedPos& other) const
{
	return NormalizedPos(m_X - other.m_X, m_Y - other.m_Y);
}
NormalizedPos NormalizedPos::operator*(const NormalizedPos& other) const
{
	return NormalizedPos(m_X * other.m_X, m_Y * other.m_Y);
}
NormalizedPos NormalizedPos::operator/(const NormalizedPos& other) const
{
	ENGINE_ASSERT(other.m_X != 0 && other.m_Y != 0, "Tried to divide a normalized position: {} "
		"by a 0 value position:{}", ToString(), other.ToString());
	return NormalizedPos(m_X / other.m_X, m_Y / other.m_Y);
}
NormalizedPos NormalizedPos::operator*(const float& scalar) const
{
	return NormalizedPos(m_X * scalar, m_Y * scalar);
}
NormalizedPos NormalizedPos::operator/(const float& scalar) const
{
	ENGINE_ASSERT(scalar != 0, "Tried to divide a normalized position: {} by a 0 value scalar", ToString());
	return NormalizedPos(m_X / scalar, m_Y / scalar);
}

NormalizedPos& NormalizedPos::operator+=(const NormalizedPos& other)
{
	m_X += other.m_X;
	m_Y += other.m_Y;
	return *this;
}
NormalizedPos& NormalizedPos::operator-=(const NormalizedPos& other)
{
	m_X -= other.m_X;
	m_Y -= other.m_Y;
	return *this;
}

bool NormalizedPos::operator==(const NormalizedPos& other) const
{
	return m_X == other.m_X && m_Y == other.m_Y;
}
bool NormalizedPos::operator!=(const NormalizedPos& other) const
{
	return !(*this == other);
}
bool NormalizedPos::operator>(const NormalizedPos& other) const
{
	return m_X > other.m_X && m_Y > other.m_Y;
}
bool NormalizedPos::operator>=(const NormalizedPos& other) const
{
	return *this > other || *this == other;
}
bool NormalizedPos::operator<(const NormalizedPos& other) const
{
	return m_X < other.m_X && m_Y < other.m_Y;
}
bool NormalizedPos::operator<=(const NormalizedPos& other) const
{
	return *this < other || *this == other;
}

NormalizedPos GetSizeFromCorners(const NormalizedPos& topLeft, const NormalizedPos& bottomRight)
{
	return NormalizedPos(bottomRight.GetX() - topLeft.GetX(), topLeft.GetY() - bottomRight.GetY());
}
NormalizedPos GetBottomRight(const NormalizedPos& topLeft, const NormalizedPos& size)
{
	return NormalizedPos(topLeft.GetX() + size.GetX(), topLeft.GetY() - size.GetY());
}
NormalizedPos GetTopLeft(const NormalizedPos& bottomRight, const NormalizedPos& size)
{
	return NormalizedPos(bottomRight.GetX() - size.GetX(), bottomRight.GetY() + size.GetY());
}