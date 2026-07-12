//NOT USED:
#include "pch.hpp"
#include "Core/Camera/CameraSettings.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::Camera
{
    CameraSettings::CameraSettings() : CameraSettings(1.0f, 20, nullptr) {}

    CameraSettings::CameraSettings(const float aspectRatio, const float lensSize, const EntityData* followTarget,
        const ProjectionType projection, const float nearDistance, const float farDistance, const float fieldOfViewYRadians)
        : m_AspectRatio(aspectRatio), m_LensSize(lensSize), m_FollowTarget(followTarget),
        m_NearDistance(nearDistance), m_FarDistance(farDistance),
        m_ProjectionType(projection), m_FieldOfViewYRadians(fieldOfViewYRadians)
    {
        if (::Math::ApproximateEqualsF(m_NearDistance, 0))
        {
            LogError(std::format("Camera settings was created with camera near distance of 0, which is not allowed"));
        }
    }

    Vec2 CameraSettings::CalculateViewportSize(const float cameraDepth) const
    {
        if (m_ProjectionType == ProjectionType::Orthographic)
            return Vec2{ m_LensSize * m_AspectRatio, m_LensSize };
        else
        {
            const float viewHeight = 2.0f * std::abs(cameraDepth) * std::tan(m_FieldOfViewYRadians * 0.5f);
            return Vec2{ viewHeight * m_AspectRatio, viewHeight };
        }
    }

    float CameraSettings::CalculateFovX() const
    {
        return 2 * std::atan(std::tan(m_FieldOfViewYRadians * 0.5f) * m_AspectRatio);
    }

    void CameraSettings::SetFieldOfViewYDegrees(const float fovY)
    {
        m_FieldOfViewYRadians = ::Math::ToRadians(fovY);
    }

    std::string CameraSettings::ToString() const
    {
        return std::format("[Apsect:{} LensSize:{} Follow:{} near:{} far:{} FovY:{} rad ]", m_AspectRatio, std::to_string(m_LensSize),
            m_FollowTarget != nullptr ? m_FollowTarget->ToString() : "NONE", m_NearDistance, m_FarDistance, m_FieldOfViewYRadians);
    }
}
