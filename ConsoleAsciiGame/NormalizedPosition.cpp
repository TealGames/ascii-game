#include "pch.hpp"
#include "NormalizedPosition.hpp"
#include "Debug.hpp"

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
//Vec2& NormalizedPosition::GetPosMutable()
//{
//	if (!Assert(this, IsValidPos(), std::format("Tried to retrieve normalized pos MUTABLE, "
//		"but it is an invalid state: {}", m_pos.ToString())))
//		throw std::invalid_argument("Invalid normalized pos state");
//
//	return m_pos;
//}

const float& NormalizedPosition::GetX() const { return m_pos.m_X; }
const float& NormalizedPosition::GetY() const { return m_pos.m_Y; }

float& NormalizedPosition::GetXMutable() { return m_pos.m_X; }
float& NormalizedPosition::GetYMutable() { return m_pos.m_Y; }

void NormalizedPosition::SetPos(const Vec2& relativePos)
{
	SetPosX(relativePos.m_X);
	SetPosY(relativePos.m_Y);
}

void NormalizedPosition::SetPosX(const float x)
{
	m_pos.m_X = std::clamp(x, float(0), float(1));
}
void NormalizedPosition::SetPosY(const float y)
{
	m_pos.m_Y = std::clamp(y, float(0), float(1));
}
void NormalizedPosition::SetPosDeltaX(const float deltaX)
{
	SetPosX(m_pos.m_X + deltaX);
}
void NormalizedPosition::SetPosDeltaY(const float deltaY)
{
	SetPosY(m_pos.m_Y + deltaY);
}

bool NormalizedPosition::IsZero() const { return Utils::ApproximateEqualsF(m_pos.m_X, 0) && Utils::ApproximateEqualsF(m_pos.m_Y, 0); }

std::string NormalizedPosition::ToString() const
{
	return std::format("{}", m_pos.ToString());
}

NormalizedPosition NormalizedPosition::operator+(const NormalizedPosition& other) const
{
	return { m_pos.m_X + other.m_pos.m_X, m_pos.m_Y + other.m_pos.m_Y };
}

NormalizedPosition NormalizedPosition::operator-(const NormalizedPosition& other) const
{
	return { m_pos.m_X - other.m_pos.m_X, m_pos.m_Y - other.m_pos.m_Y };
}

NormalizedPosition NormalizedPosition::operator*(const NormalizedPosition& other) const
{
	return { m_pos.m_X * other.m_pos.m_X, m_pos.m_Y * other.m_pos.m_Y };
}

NormalizedPosition NormalizedPosition::operator*(const float& scalar) const
{
	return { m_pos.m_X * scalar, m_pos.m_Y * scalar };
}

NormalizedPosition NormalizedPosition::operator/(const float& scalar) const
{
	if (!Assert(this, scalar != 0,
		std::format("Tried to divide a normalized position: {} by a 0 value scalar", m_pos.ToString())))
	{
		return *this;
	}

	return { m_pos.m_X / scalar, m_pos.m_Y / scalar };
}

bool NormalizedPosition::operator==(const NormalizedPosition& other) const
{
	return Utils::ApproximateEquals(m_pos.m_X, other.m_pos.m_X) &&
		Utils::ApproximateEquals(m_pos.m_Y, other.m_pos.m_Y);
}

bool NormalizedPosition::operator!=(const NormalizedPosition& other) const
{
	return !(*this == other);
}

bool NormalizedPosition::operator>(const NormalizedPosition& other) const
{
	return m_pos.m_X > other.m_pos.m_X && m_pos.m_Y > other.m_pos.m_Y;
}
bool NormalizedPosition::operator>=(const NormalizedPosition& other) const
{
	return *this > other || *this == other;
}
bool NormalizedPosition::operator<(const NormalizedPosition& other) const
{
	return m_pos.m_X < other.m_pos.m_X && m_pos.m_Y < other.m_pos.m_Y;
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