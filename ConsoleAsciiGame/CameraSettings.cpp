//NOT USED:
#include "pch.hpp"

#include "CameraSettings.hpp"
#include "EntityData.hpp"

CameraSettings::CameraSettings() : CameraSettings(Vec2Int{1,1}, 20, nullptr) {}

CameraSettings::CameraSettings(const Vec2Int& aspectRatio, const float& lensSize, const EntityData* followTarget)
    : m_AspectRatio(aspectRatio), m_LensSize(lensSize), m_WorldViewportSize(CalculateViewportSize()),
    m_FollowTarget(followTarget) {}

void CameraSettings::SetFollowNoTarget()
{
    m_FollowTarget = nullptr;
}
void CameraSettings::SetFollowTarget(const EntityData& entity)
{
    m_FollowTarget = &entity;
}

bool CameraSettings::HasNoFollowTarget() const
{
    return m_FollowTarget == nullptr;
}

WorldPosition CameraSettings::CalculateViewportSize() const
{
    return { m_LensSize, m_LensSize / m_AspectRatio.m_X * m_AspectRatio.m_Y };
}

void CameraSettings::UpdateViewportSize()
{
    m_WorldViewportSize = CalculateViewportSize();
}

std::string CameraSettings::ToString() const
{
    return std::format("[Apsect:{} LensSize:{} ViewSize:{} Follow:{}]", m_AspectRatio.ToString(), std::to_string(m_LensSize), 
        m_WorldViewportSize.ToString(), m_FollowTarget!=nullptr? m_FollowTarget->ToString() : "NONE");
}