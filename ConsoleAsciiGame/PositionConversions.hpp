#pragma once
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"

namespace Conversions
{
	Array2DPosition CartesianToArray(const CartesianPosition& pos);
	CartesianPosition ArrayToCartesian(const Array2DPosition& pos);
}
