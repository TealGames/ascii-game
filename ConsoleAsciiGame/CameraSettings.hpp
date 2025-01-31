#pragma once
#include "WorldPosition.hpp"
#include "ScreenPosition.hpp"

namespace ECS
{
	class Entity;
}

struct CameraSettings
{
	const ECS::Entity* m_FollowTarget;
	bool m_HasFixedPosition;
	WorldPosition m_WorldViewportSize;

	CameraSettings();
	CameraSettings(const WorldPosition& viewportSize, const ECS::Entity* followTarget=nullptr);
};

