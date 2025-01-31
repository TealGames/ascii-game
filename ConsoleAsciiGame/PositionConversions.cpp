#pragma once
#include "pch.hpp"
#include "CartesianPosition.hpp"
#include "Array2DPosition.hpp"
#include "PositionConversions.hpp"
#include "Entity.hpp"
#include "Vec2.hpp"

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

	ScreenPosition WorldToScreenPosition(const CameraData& camera, const WorldPosition& pos)
	{
		//TODO: could this maybe be precalculated to optimize
		float topLeftY = camera.m_Entity->m_Transform.m_Pos.m_Y + (camera.m_CameraSettings.m_WorldViewportSize.m_Y / 2);
		float bottomLeftX = camera.m_Entity->m_Transform.m_Pos.m_X - (camera.m_CameraSettings.m_WorldViewportSize.m_X / 2);
		Vec2 screenPercent{ (pos.m_X - bottomLeftX) / camera.m_CameraSettings.m_WorldViewportSize.m_X,
					 (topLeftY - pos.m_Y) / camera.m_CameraSettings.m_WorldViewportSize.m_Y };

		//TODO: right now we assume the position is for the whole screen, but we may not want the whole screen covered by the viewport
		return { static_cast<int>(screenPercent.m_X * SCREEN_WIDTH), static_cast<int>(screenPercent.m_Y * SCREEN_HEIGHT) };
	}
}