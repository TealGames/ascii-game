#include "pch.hpp"
#include "Point3D.hpp"
#include <cmath>
#include "HelperFunctions.hpp"

namespace Utils
{
	Point3D::Point3D()
		: m_X(0), m_Y(0), m_Z(0){}

	Point3D::Point3D(const float& xPos, const float& yPos, const float& zPos)
		: m_X(xPos), m_Y(yPos), m_Z(zPos){}

	inline int Point3D::XAsInt() const
	{
		return static_cast<int>(m_X);
	}

	inline int Point3D::YAsInt() const
	{
		return static_cast<int>(m_Y);
	}

	inline int Point3D::ZAsInt() const
	{
		return static_cast<int>(m_Z);
	}

	float GetDistance(const Utils::Point3D& p1, const Utils::Point3D& p2)
	{
		float bottomDistance= std::sqrtf(std::powf(p1.m_X - p2.m_X, 2) + std::powf(p1.m_Z - p2.m_Z, 2));
		return std::sqrtf(bottomDistance + std::powf(p1.m_Y - p2.m_Y, 2));
	}

	std::string Point3D::ToString() const
	{
		std::string str = std::format("({},{},{})", m_X, m_Y, m_Z);
		return str;
	}

	Point3D Point3D::operator+(const Point3D& otherPos) const
	{
		return { m_X + otherPos.m_X, m_Y + otherPos.m_Y, m_Z+ otherPos.m_Z};
	}

	Point3D Point3D::operator-(const Point3D& otherPos) const
	{
		return { m_X - otherPos.m_X, m_Y - otherPos.m_Y, m_Z- otherPos.m_Y };
	}

	Point3D Point3D::operator*(const Point3D& otherPos) const
	{
		return { m_X * otherPos.m_X, m_Y * otherPos.m_Y, m_Z* otherPos.m_Z};
	}

	Point3D Point3D::operator*(const float factor) const
	{
		return { m_X * factor, m_Y * factor, m_Z * factor};
	}

	Point3D Point3D::operator/(const Point3D& otherPos) const
	{
		if (!Utils::Assert(this, otherPos.m_X != 0 && otherPos.m_Y != 0 && otherPos.m_Z != 0,
			std::format("Tried to divide {} by a value with 0 {}",
				ToString(), otherPos.ToString()))) return {};

		return { m_X / otherPos.m_X, m_Y / otherPos.m_Y, m_Z/otherPos.m_Z};
	}

	Point3D Point3D::operator/(const float factor) const
	{
		if (!Utils::Assert(this, factor!=0, std::format("Tried to divide {} by a value with 0 {}",
				ToString(), std::to_string(factor)))) return {};

		return { m_X / factor, m_Y / factor, m_Z/factor};
	}

	bool Point3D::operator==(const Point3D& otherPos) const
	{
		return Utils::ApproximateEquals(m_X, otherPos.m_X) &&
			   Utils::ApproximateEquals(m_Y, otherPos.m_Y) &&
			   Utils::ApproximateEquals(m_Z, otherPos.m_Z);
	}

	Point3D& Point3D::operator=(const Point3D& newPos)
	{
		if (this != &newPos)
		{
			m_X = newPos.m_X;
			m_Y = newPos.m_Y;
			m_Z = newPos.m_Z;
		}
		else return *this;
	}

	Point3D& Point3D::operator=(Point3D&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_X = std::exchange(other.m_X, 0);
		m_Y = std::exchange(other.m_Y, 0);
		m_Z = std::exchange(other.m_Z, 0);

		return *this;
	}
}
