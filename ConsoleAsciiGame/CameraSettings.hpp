#pragma once
#include "Point2DInt.hpp"

namespace ECS
{
	class Entity;
}

struct CameraSettings
{
	const ECS::Entity* m_FollowTarget;
	bool m_HasFixedPosition;
	Utils::Point2DInt m_ViewportSize;

	CameraSettings();
	CameraSettings(const ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize);
	CameraSettings(const Utils::Point2DInt& viewportSize);
};

