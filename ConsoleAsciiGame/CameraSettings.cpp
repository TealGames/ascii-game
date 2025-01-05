//NOT USED:
#include "pch.hpp"

#include "CameraSettings.hpp"
#include "Entity.hpp"
#include "Point2DInt.hpp"

CameraSettings::CameraSettings() : CameraSettings({ 5, 5 }) {}

CameraSettings::CameraSettings(const ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize) :
    m_FollowTarget(&followTarget), m_HasFixedPosition(false), m_ViewportSize(viewportSize) {}

CameraSettings::CameraSettings(const Utils::Point2DInt& viewportSize) :
    m_FollowTarget(nullptr), m_HasFixedPosition(true), m_ViewportSize(viewportSize) {}

