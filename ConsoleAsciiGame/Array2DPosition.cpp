#include "pch.hpp"
#include "Array2DPosition.hpp"
#include "Point2DInt.hpp"
#include "HelperFunctions.hpp"

Array2DPosition::Array2DPosition() : 
	m_Pos() {}
Array2DPosition::Array2DPosition(const int& row, const int& col) : 
	m_Pos(row, col) {}

std::string Array2DPosition::ToString() const
{
	std::string str = std::format("({},{})", std::to_string(GetRow()), std::to_string(GetCol()));
	return str;
}

const int& Array2DPosition::GetRow() const
{
	return m_Pos.m_X;
}
const int& Array2DPosition::GetCol() const
{
	return m_Pos.m_Y;
}

void Array2DPosition::SetRow(const int& row)
{
	m_Pos.m_X = row;
}

void Array2DPosition::SetCol(const int& col)
{
	m_Pos.m_Y = col;
}

Array2DPosition Array2DPosition::operator+(const Array2DPosition& otherPos) const
{
	return { GetRow() + otherPos.GetRow(), GetCol() + otherPos.GetCol() };
}

Array2DPosition Array2DPosition::operator-(const Array2DPosition& otherPos) const
{
	return { GetRow() - otherPos.GetRow(), GetCol() - otherPos.GetCol() };
}

Array2DPosition Array2DPosition::operator*(const Array2DPosition& otherPos) const
{
	return { GetRow() * otherPos.GetRow(), GetCol() * otherPos.GetCol() };
}

Array2DPosition Array2DPosition::operator*(const int factor) const
{
	return { GetRow() * factor, GetCol() * factor };
}

Array2DPosition Array2DPosition::operator/(const Array2DPosition& otherPos) const
{
	if (!Utils::Assert(this, otherPos.GetRow() != 0 && otherPos.GetCol() != 0,
		std::format("Tried to divide {} by a value with 0 {}",
			ToString(), otherPos.ToString()))) return {};

	return { GetRow() / otherPos.GetRow(), GetCol() / otherPos.GetCol() };
}

Array2DPosition Array2DPosition::operator/(const int factor) const
{
	if (!Utils::Assert(this, factor != 0,
		std::format("Tried to divide {} by a factor with 0 {}",
			ToString(), std::to_string(factor)))) return {};

	return { GetRow() / factor, GetCol() / factor };
}

bool Array2DPosition::operator==(const Array2DPosition& otherPos) const
{
	bool sameX = Utils::ApproximateEquals(GetRow(), otherPos.GetRow());
	bool sameY = Utils::ApproximateEquals(GetCol(), otherPos.GetCol());
	return sameX && sameY;
}

bool Array2DPosition::operator!=(const Array2DPosition& otherPos) const
{
	return !(*this == otherPos);
}

bool Array2DPosition::operator<(const Array2DPosition& otherPos) const
{
	if (GetRow() == otherPos.GetRow()) return GetCol() < otherPos.GetCol();
	else return GetRow() < otherPos.GetRow();
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
		SetRow(newPos.GetRow());
		SetCol(newPos.GetCol());
	}
	return *this;
}

Array2DPosition& Array2DPosition::operator=(Array2DPosition&& other) noexcept
{
	if (this == &other)
		return *this;

	m_Pos.m_X = std::exchange(other.m_Pos.m_X, 0);
	m_Pos.m_Y = std::exchange(other.m_Pos.m_Y, 0);

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