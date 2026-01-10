#pragma once
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Utils/Data/NormalizedPosition.hpp"

enum class CoordinateOriginType : std::uint8_t
{
	/// <summary>
	/// y INCREASES DOWNWARD (V) and x INCREASES RIGHTWARD (>)
	/// </summary>
	TopLeft = 0,
	/// <summary>
	/// y INCREASES UPWARD (^) and x INCREASES RIGHTWARD (>)
	/// </summary>
	BottomLeft = 1,
};

namespace Conversions
{
	/*CartesianGridPosition ArrayToGrid(const Array2DPosition& pos);
	Array2DPosition GridToArray(const CartesianGridPosition& pos);

	CartesianGridPosition CartesianToGrid(const Vec2& pos);
	Array2DPosition CartesianToArray(const Vec2& pos);*/

	//ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos);
	//WorldPosition ScreenToWorldPosition(const CameraData& camera, const ScreenPosition& pos);

	ScreenPosition NormalizedScreenToPosition(const NormalizedPos& pos);
	NormalizedPos ScreenToNormalizedPosition(const ScreenPosition& pos, const Vec2Int totalScreenArea);

	template<typename T>
	std::optional<Vec<T, 2>> TryToNewFixedAreaPos(const Vec<T, 2> fixedArea, const CoordinateOriginType startOriginType,
		const Vec<T, 2>& pos, const CoordinateOriginType newOriginType)
	{
		if (startOriginType == newOriginType)
			return pos;

		Vec<T, 2> newPos = pos;
		if ((startOriginType == CoordinateOriginType::BottomLeft && newOriginType == CoordinateOriginType::TopLeft) ||
			(startOriginType == CoordinateOriginType::TopLeft && newOriginType == CoordinateOriginType::BottomLeft))
		{
			newPos.m_Y = fixedArea.m_Y - pos.m_Y;
		}

		if (newPos == pos) return std::nullopt;
		return newPos;
	}
}
