#pragma once
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "Point2D.hpp"

namespace Conversions
{
	CartesianGridPosition ArrayToGrid(const Array2DPosition& pos);
	Array2DPosition GridToArray(const CartesianGridPosition& pos);

	CartesianGridPosition CartesianToGrid(const Utils::Point2D& pos);
	Array2DPosition CartesianToArray(const Utils::Point2D& pos);
}
