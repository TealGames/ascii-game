#pragma once
#include "Entity.hpp"
#include "Point2DInt.hpp"

struct CameraSettings
{
	const ECS::Entity* m_FollowTarget;
	bool m_HasFixedPosition;
	Utils::Point2DInt m_ViewportSize;

	CameraSettings();
	CameraSettings(const ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize);
	CameraSettings(const Utils::Point2DInt& viewportSize);
};

