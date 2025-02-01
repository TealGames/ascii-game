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

	//Aspect ratio of the viewport in [WIDTH, HEIGHT]
	Utils::Point2D m_AspectRatio;
	/// <summary>
	/// Controls the width of the camera viewport (where height is calculated from aspect ratio)
	/// </summary>
	//TODO: controlling viewport by multiplying aspect ratio by lens size feels akward find a better way
	float m_LensSize;

	WorldPosition m_WorldViewportSize;

	CameraSettings();
	CameraSettings(const Utils::Point2D& aspectRatio, const float& lensSize, const ECS::Entity* followTarget=nullptr);
};

