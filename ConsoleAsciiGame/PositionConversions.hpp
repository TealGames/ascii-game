#pragma once
#include "CameraData.hpp"
#include "NormalizedPosition.hpp"

namespace Conversions
{
	/*CartesianGridPosition ArrayToGrid(const Array2DPosition& pos);
	Array2DPosition GridToArray(const CartesianGridPosition& pos);

	CartesianGridPosition CartesianToGrid(const Vec2& pos);
	Array2DPosition CartesianToArray(const Vec2& pos);*/

	ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos);
	WorldPosition ScreenToWorldPosition(const CameraData& camera, const ScreenPosition& pos);

	ScreenPosition NormalizedScreenToPosition(const NormalizedPosition& pos);
}
