//NOT USED:
#include "pch.hpp"
#include "Core/Camera/CameraSettings.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"

CameraSettings::CameraSettings() : CameraSettings(Vec2Int{1,1}, 20, nullptr) {}

CameraSettings::CameraSettings(const Vec2Int& aspectRatio, const float lensSize, const EntityData* followTarget, 
    const ProjectionType projection, const float nearDistance, const float farDistance, const float fieldOfViewYRadians)
    : m_AspectRatio(aspectRatio), m_LensSize(lensSize), m_FollowTarget(followTarget), 
    m_NearDistance(nearDistance), m_FarDistance(farDistance), 
    m_ProjectionType(projection), m_FieldOfViewYRadians(fieldOfViewYRadians)
{
    if (Utils::ApproximateEqualsF(m_NearDistance, 0))
    {
        LogError(std::format("Camera settings was created with camera near distance of 0, which is not allowed"));
    }
}

WorldPosition2D CameraSettings::CalculateViewportSize(const float cameraDepth) const
{
    if (m_ProjectionType== ProjectionType::Orthographic)
        return WorldPosition2D{ m_LensSize, m_LensSize / m_AspectRatio.m_X * m_AspectRatio.m_Y };
    else
    {
        const float viewHeight = 2.0f * std::abs(cameraDepth) * std::tan(m_FieldOfViewYRadians * 0.5f);
        return WorldPosition2D{ viewHeight * CalculateAspectRatio(), viewHeight};
    }
}

float CameraSettings::CalculateAspectRatio() const
{
    return static_cast<float>(m_AspectRatio.m_X) / m_AspectRatio.m_Y;
}
float CameraSettings::CalculateFovX() const
{
    return 2 * std::atan(std::tan(m_FieldOfViewYRadians * 0.5f)* CalculateAspectRatio());
}

void CameraSettings::SetFieldOfViewYDegrees(const float fovY)
{
    m_FieldOfViewYRadians = Utils::ToRadians(fovY);
}

std::string CameraSettings::ToString() const
{
    return std::format("[Apsect:{} LensSize:{} Follow:{} near:{} far:{} FovY:{} rad ]", m_AspectRatio.ToString(), std::to_string(m_LensSize), 
        m_FollowTarget!=nullptr? m_FollowTarget->ToString() : "NONE", m_NearDistance, m_FarDistance, m_FieldOfViewYRadians);
}