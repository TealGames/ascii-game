#include "pch.hpp"
#include "Array2DPosition.hpp"
#include "Point2DInt.hpp"
#include "HelperFunctions.hpp"

Array2DPosition::Array2DPosition() : 
	m_Pos(), m_Row(m_Pos.m_X), m_Col(m_Pos.m_Y) {}
Array2DPosition::Array2DPosition(const int& row, const int& col) : 
	m_Pos(row, col), m_Row(m_Pos.m_X), m_Col(m_Pos.m_Y) {}

std::string Array2DPosition::ToString() const
{
	std::string str = std::format("({},{})", m_Row, m_Col);
	return str;
}

Array2DPosition Array2DPosition::operator+(const Array2DPosition& otherPos) const
{
	return { m_Row + otherPos.m_Row, m_Col + otherPos.m_Col };
}

Array2DPosition Array2DPosition::operator-(const Array2DPosition& otherPos) const
{
	return { m_Row - otherPos.m_Row, m_Col - otherPos.m_Col };
}

Array2DPosition Array2DPosition::operator*(const Array2DPosition& otherPos) const
{
	return { m_Row * otherPos.m_Row, m_Col * otherPos.m_Col };
}

Array2DPosition Array2DPosition::operator*(const int factor) const
{
	return { m_Row * factor, m_Col * factor };
}

Array2DPosition Array2DPosition::operator/(const Array2DPosition& otherPos) const
{
	if (!Utils::Assert(this, otherPos.m_Row != 0 && otherPos.m_Col != 0,
		std::format("Tried to divide {} by a value with 0 {}",
			ToString(), otherPos.ToString()))) return {};

	return { m_Row / otherPos.m_Row, m_Col / otherPos.m_Col };
}

Array2DPosition Array2DPosition::operator/(const int factor) const
{
	if (!Utils::Assert(this, factor != 0,
		std::format("Tried to divide {} by a factor with 0 {}",
			ToString(), std::to_string(factor)))) return {};

	return { m_Row / factor, m_Col / factor };
}

bool Array2DPosition::operator==(const Array2DPosition& otherPos) const
{
	bool sameX = Utils::ApproximateEquals(m_Row, otherPos.m_Row);
	bool sameY = Utils::ApproximateEquals(m_Col, otherPos.m_Col);
	return sameX && sameY;
}

bool Array2DPosition::operator!=(const Array2DPosition& otherPos) const
{
	return !(*this == otherPos);
}

bool Array2DPosition::operator<(const Array2DPosition& otherPos) const
{
	if (m_Row == otherPos.m_Row) return m_Col < otherPos.m_Col;
	else return m_Row < otherPos.m_Row;
}
bool Array2DPosition::operator<=(const Array2DPosition& otherPos) const
{
	return *this == otherPos || *this < otherPos;
}

bool Array2DPosition::operator>(const Array2DPosition& otherPos) const
{
	return !(*this <= otherPos);
}
bool Array2DPosition::operator>=(const Array2DPosition& otherPos) const
{
	return *this == otherPos || *this > otherPos;
}

Array2DPosition& Array2DPosition::operator=(const Array2DPosition& newPos)
{
	if (this != &newPos)
	{
		m_Row = newPos.m_Row;
		m_Col = newPos.m_Col;
	}
	return *this;
}

Array2DPosition& Array2DPosition::operator=(Array2DPosition&& other) noexcept
{
	if (this == &other)
		return *this;

	m_Row = std::exchange(other.m_Row, 0);
	m_Col = std::exchange(other.m_Col, 0);

	return *this;
}

Array2DPosition::operator Utils::Point2DInt() const
{
	return m_Pos;
}

Array2DPosition GetAsArray2DPos(const Utils::Point2DInt& pos)
{
	return { pos.m_X, pos.m_Y };
}