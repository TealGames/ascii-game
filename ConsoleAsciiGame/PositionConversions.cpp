#pragma once
#include "pch.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
#include "Entity.hpp"

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

	ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos)
	{
		//TODO: could this maybe be precalculated to optimize
		float topLeftY = camera.GetEntitySafe().m_Transform.GetPos().m_Y + (camera.m_CameraSettings.m_WorldViewportSize.m_Y / 2);
		float bottomLeftX = camera.GetEntitySafe().m_Transform.GetPos().m_X - (camera.m_CameraSettings.m_WorldViewportSize.m_X / 2);
		Vec2 screenPercent{ (pos.m_X - bottomLeftX) / camera.m_CameraSettings.m_WorldViewportSize.m_X,
					 (topLeftY - pos.m_Y) / camera.m_CameraSettings.m_WorldViewportSize.m_Y };

		//TODO: right now we assume the position is for the whole screen, but we may not want the whole screen covered by the viewport
		return { static_cast<int>(screenPercent.m_X * (float)SCREEN_WIDTH), static_cast<int>(screenPercent.m_Y * (float)SCREEN_HEIGHT) };
	}


	WorldPosition ScreenToWorldPosition(const CameraData& camera, const ScreenPosition& pos)
	{
		WorldPosition bottomLeftPos = camera.GetEntitySafe().m_Transform.GetPos() - (camera.m_CameraSettings.m_WorldViewportSize / 2);
		Vec2 screenPercent{ static_cast<float>(pos.m_X) / SCREEN_WIDTH, static_cast<float>(SCREEN_HEIGHT- pos.m_Y) / SCREEN_HEIGHT};

		Vec2 worldSize = screenPercent * camera.m_CameraSettings.m_WorldViewportSize;
		return bottomLeftPos + worldSize;
	}

	ScreenPosition NormalizedScreenToPosition(const NormalizedPosition& normalized)
	{
		Vec2 screenValues = { normalized.GetPos().m_X* SCREEN_WIDTH, normalized.GetPos().m_Y * SCREEN_HEIGHT};
		return {static_cast<int>(screenValues.m_X), static_cast<int>(SCREEN_HEIGHT- screenValues.m_Y)};
	}
}