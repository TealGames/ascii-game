#pragma once
#include "Array2DPosition.hpp"
#include "Point2D.hpp"
#include "CameraData.hpp"
#include "NormalizedPosition.hpp"

namespace Conversions
{
	/*CartesianGridPosition ArrayToGrid(const Array2DPosition& pos);
	Array2DPosition GridToArray(const CartesianGridPosition& pos);

	CartesianGridPosition CartesianToGrid(const Utils::Point2D& pos);
	Array2DPosition CartesianToArray(const Utils::Point2D& pos);*/

	ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos);
	WorldPosition ScreenToWorldPosition(const CameraData& camera, const ScreenPosition& pos);

	ScreenPosition NormalizedScreenToPosition(const NormalizedPosition& pos);
}
