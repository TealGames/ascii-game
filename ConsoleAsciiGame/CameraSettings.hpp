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

	//Aspect ratio of the viewport in [WIDTH, HEIGHT]
	Vec2Int m_AspectRatio;
	/// <summary>
	/// Controls the width of the camera viewport (where height is calculated from aspect ratio)
	/// </summary>
	//TODO: controlling viewport by multiplying aspect ratio by lens size feels akward find a better way
	float m_LensSize;

	WorldPosition m_WorldViewportSize;

	CameraSettings();
	CameraSettings(const Vec2Int& aspectRatio, const float& lensSize, const ECS::Entity* followTarget=nullptr);

	bool HasFixedPosition() const;
	WorldPosition CalculateViewportSize() const;
	void UpdateViewportSize();

	void SetFollowNoTarget();
	void SetFollowTarget(const ECS::Entity& entity);

	std::string ToString() const;
};

