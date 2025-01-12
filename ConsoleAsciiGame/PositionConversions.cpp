#pragma once
#include "pch.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"

namespace Conversions
{
	Array2DPosition CartesianToArray(const CartesianPosition& pos)
	{
		return { pos.m_Y, pos.m_X };
	}

	CartesianPosition ArrayToCartesian(const Array2DPosition& pos)
	{
		return { pos.GetCol(), pos.GetRow()};
	}
}