#include "pch.hpp"
#include "Point4D.hpp"
#include "Point3D.hpp"
#include "Point2D.hpp"
#include <cmath>
#include "HelperFunctions.hpp"
#include "Debug.hpp"

namespace Utils
{
	Point4D::Point4D()
		: m_X(0), m_Y(0), m_Z(0), m_W(0) {}

	Point4D::Point4D(const float& xPos, const float& yPos, const float& zPos, const float& wPos)
		: m_X(xPos), m_Y(yPos), m_Z(zPos), m_W(wPos) {}

	inline int Point4D::XAsInt() const
	{
		return static_cast<int>(m_X);
	}

	inline int Point4D::YAsInt() const
	{
		return static_cast<int>(m_Y);
	}

	inline int Point4D::ZAsInt() const
	{
		return static_cast<int>(m_Z);
	}

	inline int Point4D::WAsInt() const
	{
		return static_cast<int>(m_W);
	}

	Utils::Point2D Point4D::ToPoint2D() const
	{
		return {m_X, m_Y};
	}
	Utils::Point3D Point4D::ToPoint3D() const
	{
		return {m_X, m_Y, m_Z};
	}

	std::string Point4D::ToString() const
	{
		std::string str = std::format("({},{},{},{})", m_X, m_Y, m_Z, m_W);
		return str;
	}

	Point4D Point4D::operator+(const Point4D& otherPos) const
	{
		return { m_X + otherPos.m_X, m_Y + otherPos.m_Y, m_Z + otherPos.m_Z, m_W+ otherPos.m_W};
	}

	Point4D Point4D::operator-(const Point4D& otherPos) const
	{
		return { m_X - otherPos.m_X, m_Y - otherPos.m_Y, m_Z - otherPos.m_Y, m_W- otherPos.m_W};
	}

	Point4D Point4D::operator*(const Point4D& otherPos) const
	{
		return { m_X * otherPos.m_X, m_Y * otherPos.m_Y, m_Z * otherPos.m_Z, m_W * otherPos.m_W};
	}

	Point4D Point4D::operator*(const float factor) const
	{
		return { m_X * factor, m_Y * factor, m_Z * factor, m_W * factor};
	}

	Point4D Point4D::operator/(const Point4D& otherPos) const
	{
		if (!Assert(this, otherPos.m_X != 0 && otherPos.m_Y != 0 && otherPos.m_Z != 0 && otherPos.m_W!=0,
			std::format("Tried to divide {} by a value with 0 {}",
				ToString(), otherPos.ToString()))) return {};

		return { m_X / otherPos.m_X, m_Y / otherPos.m_Y, m_Z / otherPos.m_Z, m_W / otherPos.m_W};
	}

	Point4D Point4D::operator/(const float factor) const
	{
		if (!Assert(this, factor != 0, std::format("Tried to divide {} by a value with 0 {}",
			ToString(), std::to_string(factor)))) return {};

		return { m_X / factor, m_Y / factor, m_Z / factor, m_W/factor};
	}

	bool Point4D::operator==(const Point4D& otherPos) const
	{
		return Utils::ApproximateEquals(m_X, otherPos.m_X) &&
			   Utils::ApproximateEquals(m_Y, otherPos.m_Y) && 
			   Utils::ApproximateEquals(m_Z, otherPos.m_Z) &&
			   Utils::ApproximateEquals(m_W, otherPos.m_W);
	}

	Point4D& Point4D::operator=(const Point4D& newPos)
	{
		if (this != &newPos)
		{
			m_X = newPos.m_X;
			m_Y = newPos.m_Y;
			m_Z = newPos.m_Z;
			m_W = newPos.m_W;
		}
		else return *this;
	}

	Point4D& Point4D::operator=(Point4D&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_X = std::exchange(other.m_X, 0);
		m_Y = std::exchange(other.m_Y, 0);
		m_Z = std::exchange(other.m_Z, 0);
		m_W = std::exchange(other.m_W, 0);

		return *this;
	}
}
