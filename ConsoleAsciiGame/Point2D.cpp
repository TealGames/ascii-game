#include "pch.hpp"
#include <cmath>
#include "Point2D.hpp"
#include "HelperFunctions.hpp"

namespace Utils
{
	Point2D::Point2D()
		: m_X(0), m_Y(0) {}

	Point2D::Point2D(float xPos, float yPos)
		: m_X(xPos), m_Y(yPos) {}

	inline int Point2D::XAsInt() const
	{
		return static_cast<int>(m_X);
	}

	inline int Point2D::YAsInt() const
	{
		return static_cast<int>(m_Y);
	}

	Utils::Point2D Point2D::GetFlipped() const
	{
		return {m_Y, m_X};
	}

	float GetDistance(const Utils::Point2D& p1, const Utils::Point2D& p2)
	{
		return std::sqrtf(std::powf(p1.m_X-p2.m_X, 2)+std::powf(p1.m_Y-p2.m_Y, 2));
	}

	std::string Point2D::ToString() const
	{
		std::string str = std::format("({},{})", m_X, m_Y);
		return str;
	}

	Point2D Point2D::operator+(const Point2D& otherPos) const
	{
		return { m_X + otherPos.m_X, m_Y + otherPos.m_Y };
	}

	Point2D Point2D::operator-(const Point2D& otherPos) const
	{
		return { m_X - otherPos.m_X, m_Y - otherPos.m_Y };
	}

	Point2D Point2D::operator*(const Point2D& otherPos) const
	{
		return { m_X * otherPos.m_X, m_Y * otherPos.m_Y };
	}

	Point2D Point2D::operator*(const float factor) const
	{
		return { m_X * factor, m_Y * factor };
	}

	Point2D Point2D::operator/(const Point2D& otherPos) const
	{
		if (otherPos.m_X == 0 || otherPos.m_Y == 0)
		{
			std::string message = std::format("Tried to divide Position {} "
				"by a pointer with 0 {}", ToString(), otherPos.ToString());
			Utils::Log(Utils::LogType::Error, message);
			return {};
		}

		return { m_X / otherPos.m_X, m_Y / otherPos.m_Y };
	}

	Point2D Point2D::operator/(const float factor) const
	{
		if (factor == 0)
		{
			std::string message = std::format("Tried to divide Position {} by factor of 0", ToString());
			Utils::Log(Utils::LogType::Error, message);
			return {};
		}

		return { m_X / factor, m_Y / factor };
	}

	bool Point2D::operator==(const Point2D& otherPos) const
	{
		bool sameX = Utils::ApproximateEquals(m_X, otherPos.m_X);
		bool sameY = Utils::ApproximateEquals(m_Y, otherPos.m_Y);
		return sameX && sameY;
	}

	Point2D& Point2D::operator=(const Point2D& newPos)
	{
		if (this != &newPos)
		{
			m_X = newPos.m_X;
			m_Y = newPos.m_Y;
		}
		else return *this;
	}

	Point2D& Point2D::operator=(Point2D&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_X = std::exchange(other.m_X, 0);
		m_Y = std::exchange(other.m_Y, 0);

		return *this;
	}
}

