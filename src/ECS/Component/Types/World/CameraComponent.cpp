#include "pch.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "StaticGlobals.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Math/PlatformMath.hpp"
#include "Math/Math3d.hpp"

namespace Engine::Camera
{
	std::string CameraPrecalculatedData::ToString() const
	{
		std::string frustumPlanesStr = "";
		for (const auto& plane : m_FrustumPlanes)
		{
			frustumPlanesStr += plane.ToString() + ",";
		}
		return std::format("[ViewMatrix:{} PlatformProjMatrix:{} EngineProjMatrix:{} FrustumPlanes:{}]",
			m_ViewMatrix.ToString(), m_PlatformProjectionMatrix.ToString(), m_EngineProjectionMatrix.ToString(),
			frustumPlanesStr);
	}

	CameraComponent::CameraComponent() : CameraComponent(CameraSettings())
	{
	}

	const CameraSettings& CameraComponent::GetSettings() const { return m_cameraSettings; }

	CameraComponent::CameraComponent(const CameraSettings& cameraSettings) :
		Component(), m_cameraSettings(cameraSettings), m_lastUpdateData()
	{
		//m_isDirty = true;
		SetAllFlagsDirty(true);
	}

	void CameraComponent::SetFollowNoTarget()
	{
		m_cameraSettings.m_FollowTarget = nullptr;
	}
	void CameraComponent::SetFollowTarget(const EntityData& entity)
	{
		m_cameraSettings.m_FollowTarget = &entity;
	}

	bool CameraComponent::HasFollowTarget() const
	{
		return m_cameraSettings.m_FollowTarget != nullptr;
	}
	const EntityData* CameraComponent::GetFollowTarget() const
	{
		return m_cameraSettings.m_FollowTarget;
	}

	void CameraComponent::SetNearDistance(const float near)
	{
		if (near > m_cameraSettings.m_FarDistance)
		{
			LogWarning(std::format("Attempted to set camera:{} near distance:{} greater than far distance:{}",
				GetEntity().ToString(), near, m_cameraSettings.m_FarDistance));
			return;
		}
		m_cameraSettings.m_NearDistance = std::abs(near);
	}
	void CameraComponent::SetFarDistance(const float far)
	{
		if (far < m_cameraSettings.m_NearDistance)
		{
			LogWarning(std::format("Attempted to set camera:{} far distance:{} less than near distance:{}",
				GetEntity().ToString(), far, m_cameraSettings.m_NearDistance));
			return;
		}
		m_cameraSettings.m_FarDistance = std::abs(far);
	}

	float CameraComponent::GetNearDistance() const { return m_cameraSettings.m_NearDistance; }
	float CameraComponent::GetFarDistance() const { return m_cameraSettings.m_FarDistance; }
	float CameraComponent::GetViewDistance() const { return GetFarDistance() - GetNearDistance(); }

	Vec3 CameraComponent::WorldToNdcPosition(const WorldPosition3D& pos, const ProjectionMatrixType matrixType) const
	{
		const Vec4 clipPos = (matrixType == ProjectionMatrixType::Engine ? GetLastUpdateData().m_EngineProjectionMatrix
			: GetLastUpdateData().m_PlatformProjectionMatrix) * GetLastUpdateData().m_ViewMatrix * Vec4(pos, 1);

		if (::Math::ApproximateEqualsF(clipPos.m_W, 0))
			return INVALID_NDC_POS;
		return clipPos.GetXYZ() / clipPos.m_W;

	}

	ScreenPosition CameraComponent::WorldToScreenPosition(const WorldPosition3D& pos) const
	{
		const Vec4 clipPos = GetLastUpdateData().m_EngineProjectionMatrix * GetLastUpdateData().m_ViewMatrix * Vec4(pos, 1);

		//This means we are outside view and cannot flatten 4d pos -> 3d
		if (::Math::ApproximateEqualsF(clipPos.m_W, 0))
		{
			LogError(std::format("Attempted to convert world pos:{} (clip pos:{}) to screen using camera:{}"
				"but that world pos is outside the camera view and thus cannot be mapped to a valid screen position. "
				"View area at its z depth:{}", pos.ToString(), clipPos.ToString(), GetTransform().ToString(),
				m_cameraSettings.CalculateViewportSize(pos.m_Z - m_cameraSettings.m_NearDistance).ToString()));
			return {};
		}

		Vec3 ndcPos = clipPos.GetXYZ() / clipPos.m_W;
		for (size_t i = 0; i < 3; i++)
		{
			if (ndcPos[i] > 1 || (ENGINE_NDC_RANGES[i] == NdcRange::NegOneToOne && ndcPos[i] < -1) ||
				(ENGINE_NDC_RANGES[i] == NdcRange::ZeroToOne && ndcPos[i] < 0))
			{
				LogError(std::format("Attempted to convert world pos:{} (ndc pos:{}) to screen using camera:{}"
					"but that world pos is outside the camera view and thus cannot be mapped to a valid screen position. "
					"View Area at itsz depth:{}", pos.ToString(), ndcPos.ToString(), GetTransform().ToString(),
					m_cameraSettings.CalculateViewportSize(pos.m_Z - m_cameraSettings.m_NearDistance).ToString()));
				return {};
			}
		}

		//This ensures that we have x and y normalized from 0 to 1
		if (ENGINE_NDC_RANGES[0] == NdcRange::NegOneToOne) ndcPos.m_X = (ndcPos.m_X + 1) * 0.5;
		if (ENGINE_NDC_RANGES[1] == NdcRange::NegOneToOne) ndcPos.m_Y = (ndcPos.m_Y + 1) * 0.5;
		//Note: since we use top left as origin for screen pos (and y increases down), we have to invert y pos
		return ScreenPosition(ndcPos.m_X * SCREEN_WIDTH, (1 - ndcPos.m_Y) * SCREEN_HEIGHT);
	}
	Math::Ray3D CameraComponent::ScreenToWorldPosition(const ScreenPosition& pos) const
	{
		const WorldPosition3D cameraPos = GetEntity().GetTransform().GetWorldPos();
		const Vec2 nearSize = m_cameraSettings.CalculateViewportSize(m_cameraSettings.m_NearDistance);
		const Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };
		const float camX = (screenPercent.m_X - 0.5f) * nearSize.m_X;
		const float camY = (0.5f - screenPercent.m_Y) * nearSize.m_Y;

		const Vec3 nearPoint = cameraPos + GetTransform().CalculateWorldForward() * m_cameraSettings.m_NearDistance +
			GetTransform().CalculateWorldRight() * camX + GetTransform().CalculateWorldUp() * camY;

		Vec3 dir = {};
		WorldPosition3D origin = {};
		if (m_cameraSettings.m_ProjectionType == ProjectionType::Orthographic)
		{
			origin = nearPoint;
			dir = GetTransform().CalculateWorldForward();
		}
		else
		{
			origin = cameraPos;
			dir = (nearPoint - cameraPos).GetNormalized();
		}
		return Math::Ray3D(origin, dir * GetViewDistance());
	}
	Vec2 CameraComponent::WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const
	{
		//if orthographic, since the size does not change as you move away, we just get fraction of world
		//object to viewport area within screen and we do not care about depth (Z axis) since it stays the same no matter the depth
		if (m_cameraSettings.m_ProjectionType == ProjectionType::Orthographic)
			return worldSize.GetXY() / m_cameraSettings.CalculateViewportSize() * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
		else
		{
			const Vec2 frustrumSize = m_cameraSettings.CalculateViewportSize(cameraDepthDistance);
			return worldSize.GetXY() / frustrumSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
		}
	}

	WorldPosition3D CameraComponent::CalculateNearPlaneWorldCenter() const
	{
		return GetTransform().GetWorldPos() + (GetTransform().CalculateWorldForward() * m_cameraSettings.m_NearDistance);
	}
	WorldPosition3D CameraComponent::CalculateFarPlaneWorldCenter() const
	{
		return GetTransform().GetWorldPos() + (GetTransform().CalculateWorldForward() * m_cameraSettings.m_FarDistance);
	}

	std::array<Math::InfinitePlane3D, 6> CameraComponent::CalculateFrustumPlanes() const
	{
		//Note: here we must use data directly not via function to prevent recursive loop + 
		//since we calculate this last, we are guaranteed to have the most updated proj and view matrix
		const Mat4 viewProjection = m_lastUpdateData.m_EngineProjectionMatrix * m_lastUpdateData.m_ViewMatrix;

		/// LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
		std::array<Math::InfinitePlane3D, 6> planes = {};
		planes[0] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(0) };
		planes[1] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(0) };
		planes[2] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(1) };
		planes[3] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(1) };
		planes[4] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(2) };
		planes[5] = Math::InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(2) };

		for (auto& plane : planes)
			plane.NormalizeNormal();

		return planes;
	}

	bool CameraComponent::DoesViewVolumeContainPos(const WorldPosition3D& point) const
	{
		for (const auto& plane : GetLastUpdateData().m_FrustumPlanes)
		{
			//Note: the furstum planes have normals that point inward, so "forward" is considering inside frustum
			//and back of plane is considered outside view area
			if (plane.IsPointBackOfPlane(point))
				return false;
		}
		return true;
	}

	Mat4 CalculateViewMatrix(const WorldPosition3D& globalPos, const Math::Quat& globalRotation)
	{
		// For column-major: V = R^T * T(-pos)
		// But we need to rotate the translation, so: V = [R^T | -R^T * pos]

		const Mat4 rotationMatrix = Math::CalculateRotationMatrix(globalRotation);
		const Mat4 invertedRotation = rotationMatrix.Transpose();

		// Apply inverted rotation to the NEGATED position (w=0 to ignore translation)
		const Vec3 transformedPos = (invertedRotation * Vec4(-globalPos, 0.0f)).GetXYZ();

		// Build view matrix with the transformed translation
		Mat4 viewMatrix = invertedRotation;
		viewMatrix.SetCol(3, Vec4(transformedPos, 1.0f));
		return viewMatrix;

	}
	Mat4 CalculateViewMatrix(const WorldPosition3D& globalPos, const Vec3& forwardDir, const Vec3& upDir)
	{
		//Note: technically we do not need forward dir normalized (since arg should be that way) and updir created from forward and right 
		//since we have up dir, but it is to ensure no floating point imprecission that might mess up calculations
		const Vec3 forwardDirNormalized = forwardDir.GetNormalized();
		const Vec3 rightDir = Math::CrossProduct(upDir, forwardDirNormalized).GetNormalized();
		const Vec3 upDirSafe = Math::CrossProduct(forwardDirNormalized, rightDir);

		Mat4 rot = Mat4::GetIdentity();
		rot.SetCol(0, Vec4(rightDir, 0));
		rot.SetCol(1, Vec4(upDirSafe, 0.0f));
		rot.SetCol(2, Vec4(-forwardDirNormalized, 0.0f));

		return rot * Math::CalculateTranslationMatrix(-globalPos);
	}

	Mat4 CameraComponent::CalculateProjectionMatrix(const ProjectionMatrixType type) const
	{
		const float zNear = m_cameraSettings.m_NearDistance;
		const float zFar = m_cameraSettings.m_FarDistance;
		if (m_cameraSettings.m_ProjectionType == ProjectionType::Perspective)
		{
			if (type == ProjectionMatrixType::Engine)
			{
				return Math::CalculatePerspectiveProjMatrix(ENGINE_NDC_RANGES[2], ENGINE_FORWARD_SIGN_Z, m_cameraSettings.m_FieldOfViewYRadians,
					m_cameraSettings.m_AspectRatio, zNear, zFar);
			}

			return Math::Platforms::CalculatePlatformPerspectiveProjMatrix(m_cameraSettings.m_FieldOfViewYRadians,
				m_cameraSettings.m_AspectRatio, zNear, zFar);
		}
		else
		{
			const WorldPosition3D globalPos = GetTransform().GetWorldPos();
			const Vec2 viewportSize = m_cameraSettings.CalculateViewportSize();
			const float l = globalPos.m_X - viewportSize.m_X / 2;
			const float r = globalPos.m_X + viewportSize.m_X / 2;
			const float b = globalPos.m_Y - viewportSize.m_Y / 2;
			const float t = globalPos.m_Y + viewportSize.m_Y / 2;

			if (type == ProjectionMatrixType::Engine)
				return Math::CalculateOrthographicProjMatrix(ENGINE_NDC_RANGES[2], ENGINE_FORWARD_SIGN_Z, r, l, t, b, zNear, zFar);

			return Math::Platforms::CalculatePlatformOrthographicProjMatrix(r, l, t, b, zNear, zFar);
		}
	}

	const CameraPrecalculatedData& CameraComponent::GetLastUpdateData() const
	{
		const bool viewMatrixDirty = HasDirtyFlag(VIEW_MATRIX_DIRTY_FLAG);
		const bool projMatrixDirty = HasDirtyFlag(PROJ_MATRIX_DIRTY_FLAG);
		if (viewMatrixDirty)
		{
			m_lastUpdateData.m_ViewMatrix = CalculateViewMatrix(GetTransform().GetWorldPos(), GetTransform().GetWorldRotation());
			m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::ViewMatrix;
		}
		if (projMatrixDirty)
		{
			m_lastUpdateData.m_PlatformProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Platform);
			m_lastUpdateData.m_EngineProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Engine);
			m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::EngineProjMatrix | CameraPrecalculatedDataUpdate::PlatformProjMatrix;
		}
		if (viewMatrixDirty || projMatrixDirty)
		{
			m_lastUpdateData.m_FrustumPlanes = CalculateFrustumPlanes();
			m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::FrustumPlanes;
		}
		SetAllFlagsDirty(false);
		return m_lastUpdateData;
	}

	void CameraComponent::InitFields()
	{
		m_Fields = { ECS::ComponentField("LensSize", &m_cameraSettings.m_LensSize) };
	}
	void CameraComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("AspectRatio", m_cameraSettings.m_AspectRatio);
		serializer.AddProperty("LensSize", m_cameraSettings.m_LensSize);
		serializer.AddProperty("FollowTarget", Serialization::TrySerializeEntity(m_cameraSettings.m_FollowTarget, true));
	}
	void CameraComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("AspectRatio", &m_cameraSettings.m_AspectRatio);
		deserializer.GetProperty("LensSize", &m_cameraSettings.m_LensSize);

		std::optional<Serialization::SerializedEntity> serializedEntity = std::nullopt;
		deserializer.GetProperty("FollowTarget", &serializedEntity);
		EntityData* maybeFollowTarget = Serialization::TryDeserializeEntity(serializedEntity, true);

		if (maybeFollowTarget != nullptr) SetFollowTarget(*maybeFollowTarget);
		else SetFollowNoTarget();

		SetAllFlagsDirty(true);
	}

	std::string CameraComponent::ToString() const
	{
		return std::format("[Camera Settings:{}]",
			m_cameraSettings.ToString());
	}
}