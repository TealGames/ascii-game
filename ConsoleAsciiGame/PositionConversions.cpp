#pragma once
#include "pch.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"

namespace Conversions
{
	Array2DPosition GridToArray(const CartesianGridPosition& pos)
	{
		return { pos.m_Y, pos.m_X };
	}

	CartesianGridPosition ArrayToGrid(const Array2DPosition& pos)
	{
		return { pos.GetCol(), pos.GetRow() };
	}

	CartesianGridPosition CartesianToGrid(const Utils::Point2D& pos)
	{
		return { static_cast<int>(std::floorf(pos.m_X)), 
				 static_cast<int>(std::floorf(pos.m_Y)) };
	}

	Array2DPosition CartesianToArray(const Utils::Point2D& pos)
	{
		return GridToArray(CartesianToGrid(pos));
	}
}