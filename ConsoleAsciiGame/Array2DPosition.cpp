#include "pch.hpp"
#include "Array2DPosition.hpp"

Array2DPosition::Array2DPosition() : 
	Utils::Point2DInt(), m_Row(m_X), m_Col(m_Y) {}
Array2DPosition::Array2DPosition(const int& row, const int& col) : 
	Utils::Point2DInt(row, col), m_Row(m_X), m_Col(m_Y) {}

std::string Array2DPosition::ToString() const
{
	std::string str = std::format("({},{})", m_Row, m_Col);
	return str;
}
