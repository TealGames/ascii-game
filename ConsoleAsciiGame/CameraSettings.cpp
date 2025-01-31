//NOT USED:
#include "pch.hpp"

#include "CameraSettings.hpp"
#include "Entity.hpp"
#include "Point2DInt.hpp"

CameraSettings::CameraSettings() : CameraSettings(WorldPosition{ 5, 5 }, nullptr) {}

CameraSettings::CameraSettings(const WorldPosition& viewportSize, 
    const ECS::Entity* followTarget) :
    m_WorldViewportSize(viewportSize), 
    m_FollowTarget(followTarget), m_HasFixedPosition(m_FollowTarget==nullptr) {}


