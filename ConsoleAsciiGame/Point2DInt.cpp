#include "pch.hpp"
#include "Point2DInt.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

namespace Utils
{
	const Point2DInt Point2DInt::ONE = { 1, 1 };
	const Point2DInt Point2DInt::ZERO = { 0, 0 };

	constexpr Point2DInt::Point2DInt()
		:m_X(0), m_Y(0) {}

	constexpr Point2DInt::Point2DInt(const int& xPos, const int& yPos)
		: m_X(xPos), m_Y(yPos) {}

	std::string Point2DInt::ToString() const
	{
		std::string str = std::format("({},{})", m_X, m_Y);
		return str;
	}

	Utils::Point2DInt Point2DInt::GetFlipped() const
	{
		return {m_Y, m_X};
	}

	int GetDistance(const Utils::Point2DInt& p1, const Utils::Point2DInt& p2)
	{
		return std::sqrt(std::pow(p1.m_X - p2.m_X, 2) + std::pow(p1.m_Y - p2.m_Y, 2));
	}

	Point2DInt Point2DInt::operator+(const Point2DInt& otherPos) const
	{
		return { m_X + otherPos.m_X, m_Y + otherPos.m_Y };
	}

	Point2DInt Point2DInt::operator-(const Point2DInt& otherPos) const
	{
		return { m_X - otherPos.m_X, m_Y - otherPos.m_Y };
	}

	Point2DInt Point2DInt::operator*(const Point2DInt& otherPos) const
	{
		return { m_X * otherPos.m_X, m_Y * otherPos.m_Y };
	}

	Point2DInt Point2DInt::operator*(const int factor) const
	{
		return { m_X * factor, m_Y * factor };
	}

	Point2DInt Point2DInt::operator/(const Point2DInt& otherPos) const
	{
		if (otherPos.m_X == 0 || otherPos.m_Y == 0)
		{
			std::string message = std::format("Tried to divide Position {} "
				"by a pointer with 0 {}", ToString(), otherPos.ToString());
			LogError(this, message);
			return {};
		}

		return { m_X / otherPos.m_X, m_Y / otherPos.m_Y };
	}

	Point2DInt Point2DInt::operator/(const int factor) const
	{
		if (factor == 0)
		{
			std::string message = std::format("Tried to divide Position {} by factor of 0", ToString());
			LogError(this, message);
			return {};
		}

		return { m_X / factor, m_Y / factor };
	}

	bool Point2DInt::operator==(const Point2DInt& otherPos) const
	{
		bool sameX = Utils::ApproximateEquals(m_X, otherPos.m_X);
		bool sameY = Utils::ApproximateEquals(m_Y, otherPos.m_Y);
		return sameX && sameY;
	}

	bool Point2DInt::operator!=(const Point2DInt& otherPos) const
	{
		return !(*this == otherPos);
	}

	bool Point2DInt::operator<(const Point2DInt& otherPos) const
	{
		if (m_X == otherPos.m_X) return m_Y < otherPos.m_Y;
		else return m_X < otherPos.m_X;
	}
	bool Point2DInt::operator<=(const Point2DInt& otherPos) const
	{
		return *this == otherPos || *this < otherPos;
	}

	bool Point2DInt::operator>(const Point2DInt& otherPos) const
	{
		return !(*this <= otherPos);
	}
	bool Point2DInt::operator>=(const Point2DInt& otherPos) const
	{
		return *this == otherPos || *this > otherPos;
	}

	Point2DInt& Point2DInt::operator=(const Point2DInt& newPos)
	{
		if (this != &newPos)
		{
			m_X = newPos.m_X;
			m_Y= newPos.m_Y;
		}
		return *this;
	}

	Point2DInt& Point2DInt::operator=(Point2DInt&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_X = std::exchange(other.m_X, 0);
		m_Y = std::exchange(other.m_Y, 0);

		return *this;
	}
}

