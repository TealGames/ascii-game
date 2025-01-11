#pragma once
#include "Point2DInt.hpp"

struct Array2DPosition final : public Utils::Point2DInt
{
	int& m_Row;
	int& m_Col;

	Array2DPosition();
	Array2DPosition(const int& row, const int& col);
	Array2DPosition(const Array2DPosition&) = default;

	std::string ToString() const override;
};