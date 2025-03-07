//NOT USED:
#include "pch.hpp"

#include "CameraSettings.hpp"
#include "Entity.hpp"

CameraSettings::CameraSettings() : CameraSettings(Vec2Int{1,1}, 20, nullptr) {}

CameraSettings::CameraSettings(const Vec2Int& aspectRatio, const float& lensSize, const ECS::Entity* followTarget)
    : m_AspectRatio(aspectRatio), m_LensSize(lensSize), m_WorldViewportSize(lensSize, lensSize/ m_AspectRatio.m_X* m_AspectRatio.m_Y),
    m_FollowTarget(followTarget), m_HasFixedPosition(m_FollowTarget==nullptr) {}


