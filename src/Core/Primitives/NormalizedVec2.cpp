#include "pch.hpp"
#include "Core/Primitives/NormalizedVec2.hpp"
#include "Utils/Debug.hpp"

namespace Engine
{
	NormalizedVec2::NormalizedVec2() :
		NormalizedVec2(0, 0) {}

	NormalizedVec2::NormalizedVec2(const Vec2& pos) :
		NormalizedVec2(pos.m_X, pos.m_Y) {}

	NormalizedVec2::NormalizedVec2(const float& x, const float& y) : m_X(x), m_Y(y) {}
	NormalizedVec2::NormalizedVec2(const NormalizedValue x, const NormalizedValue y)
		: m_X(x), m_Y(y) {}

	NormalizedVec2::NormalizedVec2(const float x, const NormalizedValue y) : NormalizedVec2(NormalizedValue(x), y) {}
	NormalizedVec2::NormalizedVec2(const NormalizedValue x, const float y) : NormalizedVec2(x, NormalizedValue(y)) {}

	Vec2 NormalizedVec2::AsVec2() const
	{
		return Vec2(m_X.GetValue(), m_Y.GetValue());
	}
	float NormalizedVec2::GetX() const { return m_X.GetValue(); }
	float NormalizedVec2::GetY() const { return m_Y.GetValue(); }

	void NormalizedVec2::SetPos(const Vec2& relativePos)
	{
		/*SetPosX(relativePos.m_X);
		SetPosY(relativePos.m_Y);*/
		m_X = relativePos.m_X;
		m_Y = relativePos.m_Y;
	}

	bool NormalizedVec2::IsZero() const
	{
		return m_X == 0 && m_Y == 0;
	}

	std::string NormalizedVec2::ToString() const
	{
		return std::format("({},{})", m_X.ToString(), m_Y.ToString());
	}

	NormalizedVec2 NormalizedVec2::operator+(const NormalizedVec2& other) const
	{
		return NormalizedVec2(m_X + other.m_X, m_Y + other.m_Y);
	}
	NormalizedVec2 NormalizedVec2::operator-(const NormalizedVec2& other) const
	{
		return NormalizedVec2(m_X - other.m_X, m_Y - other.m_Y);
	}
	NormalizedVec2 NormalizedVec2::operator*(const NormalizedVec2& other) const
	{
		return NormalizedVec2(m_X * other.m_X, m_Y * other.m_Y);
	}
	NormalizedVec2 NormalizedVec2::operator/(const NormalizedVec2& other) const
	{
		ENGINE_ASSERT(other.m_X != 0 && other.m_Y != 0, "Tried to divide a normalized position: {} "
			"by a 0 value position:{}", ToString(), other.ToString());
		return NormalizedVec2(m_X / other.m_X, m_Y / other.m_Y);
	}
	NormalizedVec2 NormalizedVec2::operator*(const float& scalar) const
	{
		return NormalizedVec2(m_X * scalar, m_Y * scalar);
	}
	NormalizedVec2 NormalizedVec2::operator/(const float& scalar) const
	{
		ENGINE_ASSERT(scalar != 0, "Tried to divide a normalized position: {} by a 0 value scalar", ToString());
		return NormalizedVec2(m_X / scalar, m_Y / scalar);
	}

	NormalizedVec2& NormalizedVec2::operator+=(const NormalizedVec2& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		return *this;
	}
	NormalizedVec2& NormalizedVec2::operator-=(const NormalizedVec2& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		return *this;
	}

	bool NormalizedVec2::operator==(const NormalizedVec2& other) const
	{
		return m_X == other.m_X && m_Y == other.m_Y;
	}
	bool NormalizedVec2::operator!=(const NormalizedVec2& other) const
	{
		return !(*this == other);
	}
	bool NormalizedVec2::operator>(const NormalizedVec2& other) const
	{
		return m_X > other.m_X && m_Y > other.m_Y;
	}
	bool NormalizedVec2::operator>=(const NormalizedVec2& other) const
	{
		return *this > other || *this == other;
	}
	bool NormalizedVec2::operator<(const NormalizedVec2& other) const
	{
		return m_X < other.m_X && m_Y < other.m_Y;
	}
	bool NormalizedVec2::operator<=(const NormalizedVec2& other) const
	{
		return *this < other || *this == other;
	}
}
