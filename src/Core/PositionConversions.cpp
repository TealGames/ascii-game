#pragma once
#include "pch.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "Core/PositionConversions.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "StaticGlobals.hpp"

namespace Conversions
{
	/*Array2DPosition GridToArray(const CartesianGridPosition& pos)
	{
		return { pos.m_Y, pos.m_X };
	}

	CartesianGridPosition ArrayToGrid(const Array2DPosition& pos)
	{
		return { pos.GetCol(), pos.GetRow() };
	}

	CartesianGridPosition CartesianToGrid(const Vec2& pos)
	{
		return { static_cast<int>(std::floorf(pos.m_X)), 
				 static_cast<int>(std::floorf(pos.m_Y)) };
	}

	Array2DPosition CartesianToArray(const Vec2& pos)
	{
		return GridToArray(CartesianToGrid(pos));
	}*/

	/*ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos)
	{
		
	}*/

	/*
	WorldPosition3D ScreenToWorldPosition(const CameraData& camera, const ScreenPosition& pos)
	{
		WorldPosition3D bottomLeftPos = camera.GetEntity().GetTransform().GetGlobalPos() - (camera.m_CameraSettings.m_WorldViewportSize / 2);
		Vec2 screenPercent{ static_cast<float>(pos.m_X) / SCREEN_WIDTH, static_cast<float>(SCREEN_HEIGHT- pos.m_Y) / SCREEN_HEIGHT};

		Vec2 worldSize = screenPercent * camera.m_CameraSettings.m_WorldViewportSize;
		return bottomLeftPos + worldSize;
	}
	*/

	ScreenPosition NormalizedScreenToPosition(const NormalizedPos& normalized)
	{
		Vec2 screenValues = { normalized.AsVec2().m_X* SCREEN_WIDTH, normalized.AsVec2().m_Y * SCREEN_HEIGHT};
		return ScreenPosition{screenValues.m_X, SCREEN_HEIGHT- screenValues.m_Y};
	}

	NormalizedPos ScreenToNormalizedPosition(const ScreenPosition& pos, const Vec2Int totalScreenArea)
	{
		return { static_cast<float>(pos.m_X) / totalScreenArea.m_X, float(1) - (static_cast<float>(pos.m_Y) / totalScreenArea.m_Y) };
	}
}