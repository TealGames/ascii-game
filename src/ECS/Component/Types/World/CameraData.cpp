#include "pch.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/PositionConversions.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Data/Quaternion.hpp"
#include "Utils/Data/Vec4Type.hpp"
#include "Math/PlatformMath.hpp"

CameraComponent::CameraComponent() : CameraComponent(CameraSettings()) {}

CameraComponent::CameraComponent(const Json& json) : CameraComponent()
{
	Deserialize(json);
}

CameraComponent::CameraComponent(const CameraSettings& cameraSettings) :
	Component(), m_cameraSettings(cameraSettings), m_lastUpdateData() {}

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

ScreenPosition CameraComponent::WorldToScreenPosition(const WorldPosition3D& pos) const
{
	//Note: we must use 4d vector for matrix multiplication -> 1 for w just means it is a point (0 would mean direction)
	const Vec4 clipPos = m_lastUpdateData.m_EngineProjectionMatrix * m_lastUpdateData.m_ViewMatrix * Vec4(pos, 1);
	Vec3 ndcPos = clipPos.GetXYZ() / clipPos.m_W;
	//This ensures that we have x and y normalized from 0 to 1
	if (ENGINE_NDC_RANGE_X == NdcRange::NegOneToOne) ndcPos.m_X = (ndcPos.m_X + 1) * 0.5;
	if (ENGINE_NDC_RANGE_Y == NdcRange::NegOneToOne) ndcPos.m_Y = (ndcPos.m_Y + 1) * 0.5;

	//Note: since we use top left as origin for screen pos (and y increases down), we have to invert y pos
	return ScreenPosition(ndcPos.m_X * SCREEN_WIDTH, (1- ndcPos.m_Y)*SCREEN_HEIGHT);
}
Ray3D CameraComponent::ScreenToWorldPosition(const ScreenPosition& pos) const
{
	const WorldPosition3D cameraPos = GetEntity().GetTransform().GetGlobalPos();
	const WorldPosition2D bottomLeftPos = cameraPos.GetXY() - (m_cameraSettings.m_WorldViewportSize / 2);
	Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };

	Vec2 worldSize = screenPercent * m_cameraSettings.m_WorldViewportSize;
	//LogWarning(std::format("CAMERA SHIT coord:{} screen percetn:{} viewport size:{}", pos.ToString(), screenPercent.ToString(), m_CameraSettings.m_WorldViewportSize.ToString()));
	return Ray3D(WorldPosition3D(bottomLeftPos + worldSize, CalculateNearPlaneWorldCenter().m_Z), Vec3(0, 0, GetViewDistance()));
}
Vec2 CameraComponent::WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const
{
	//if orthographic, since the size does not change as you move away, we just get fraction of world
	//object to viewport area within screen and we do not care about depth (Z axis) since it stays the same no matter the depth
	if (m_cameraSettings.m_ProjectionType== ProjectionType::Orthographic)	
		return worldSize.GetXY() / m_cameraSettings.m_WorldViewportSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	else
	{
		const Vec2 frustrumSize = m_cameraSettings.CalculateViewportSize(cameraDepthDistance);
		return worldSize.GetXY() / frustrumSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
}

Vec3 CameraComponent::CalculateWorldForward() const
{
	return GetTransform().GetGlobalRotation().ApplyRotationToDir(ENGINE_FORWARD_DIR);
}

WorldPosition3D CameraComponent::CalculateNearPlaneWorldCenter() const
{
	return GetTransform().GetGlobalPos() + (CalculateWorldForward() * m_cameraSettings.m_NearDistance);
}
WorldPosition3D CameraComponent::CalculateFarPlaneWorldCenter() const
{
	return GetTransform().GetGlobalPos() + (CalculateWorldForward() * m_cameraSettings.m_FarDistance);
}

std::array<InfinitePlane3D, 6> CameraComponent::CalculateFrustumPlanes() const
{
	const Mat4 viewProjection = m_lastUpdateData.m_EngineProjectionMatrix * m_lastUpdateData.m_ViewMatrix;

	/// LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
	std::array<InfinitePlane3D, 6> planes = {};
	planes[0] = InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(0) };
	planes[1] = InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(0) };
	planes[2] = InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(1) };
	planes[3] = InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(1) };
	planes[4] = InfinitePlane3D{ viewProjection.GetRowVector(3) + viewProjection.GetRowVector(2) };
	planes[5] = InfinitePlane3D{ viewProjection.GetRowVector(3) - viewProjection.GetRowVector(2) };

	for (auto& plane : planes)
		plane.NormalizeNormal();

	return planes;
}

bool CameraComponent::DoesViewVolumeContainPos(const WorldPosition3D& point) const
{
	for (const auto& plane : m_lastUpdateData.m_FrustumPlanes)
	{
		//Note: the furstum planes have normals that point inward, so "forward" is considering inside frustum
		//and back of plane is considered outside view area
		if (plane.IsPointBackOfPlane(point))
			return false;
	}
	return true;
}

Mat4 CameraComponent::CalculateViewMatrix() const
{
	//Since the rotation matrix is a special kind of matrix its inverse == tranpose (this is not normally true)
	const Mat4 invertedRotationMatrix = TransformComponent::CalculateRotationMatrix(GetTransform().GetGlobalRotation()).Transpose();
	const Vec4 rotatedTranslation = invertedRotationMatrix * Vec4(-GetTransform().GetGlobalPos(), 1.0f);
	return invertedRotationMatrix * TransformComponent::CalculateTranslationMatrix(rotatedTranslation.GetXYZ());
}

Mat4 CameraComponent::CalculateProjectionMatrix(const ProjectionMatrixType type) const
{
	const float zNear = m_cameraSettings.m_NearDistance;
	const float zFar = m_cameraSettings.m_FarDistance;
	if (m_cameraSettings.m_ProjectionType == ProjectionType::Perspective)
	{
		if (type == ProjectionMatrixType::Engine)
		{
			return PlatformMath::CalculatePerspectiveViewMatrix(ENGINE_NDC_RANGE_Z, ENGINE_FORWARD_SIGN_Z, m_cameraSettings.m_FieldOfViewYRadians,
				m_cameraSettings.CalculateAspectRatio(), zNear, zFar);
		}

		return PlatformMath::CalculatePlatformPerspectiveViewMatrix(m_cameraSettings.m_FieldOfViewYRadians, 
			m_cameraSettings.CalculateAspectRatio(), zNear, zFar);
	}
	else
	{
		const WorldPosition3D globalPos = GetTransform().GetGlobalPos();
		const float l = globalPos.m_X - m_cameraSettings.m_WorldViewportSize.m_X / 2;
		const float r = globalPos.m_X + m_cameraSettings.m_WorldViewportSize.m_X / 2;
		const float b = globalPos.m_Y - m_cameraSettings.m_WorldViewportSize.m_Y / 2;
		const float t = globalPos.m_Y + m_cameraSettings.m_WorldViewportSize.m_Y / 2;

		if (type == ProjectionMatrixType::Engine)
			PlatformMath::CalculateOrthographicViewMatrix(ENGINE_NDC_RANGE_Z, ENGINE_FORWARD_SIGN_Z, r, l, t, b, zNear, zFar);

		return PlatformMath::CalculatePlatformOrthographicViewMatrix(r, l, t, b, zNear, zFar);
	}
}

void CameraComponent::UpdatePrecalculatedData()
{
	//TODO: do we need platform or engine frustum planes?
	m_lastUpdateData.m_ViewMatrix = CalculateViewMatrix();
	m_lastUpdateData.m_PlatformProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Platform);
	m_lastUpdateData.m_EngineProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Engine);
	m_lastUpdateData.m_FrustumPlanes = CalculateFrustumPlanes();
}
const CameraPrecalculatedData& CameraComponent::GetLastUpdateData() const { return m_lastUpdateData; }

void CameraComponent::InitFields()
{
	m_Fields= { ComponentField("LensSize", &m_cameraSettings.m_LensSize) };
}
//std::vector<std::string> CameraData::GetDependencyFlags() const
//{
//	return {ENTITY_DEPENDENCY_FLAG};
//}

std::string CameraComponent::ToString() const
{
	return std::format("[Camera Settings:{}]", 
		m_cameraSettings.ToString());
}

void CameraComponent::Deserialize(const Json& json)
{
	m_cameraSettings.m_AspectRatio = json.at("AspectRatio").get<Vec2Int>();
	m_cameraSettings.m_LensSize = json.at("LensSize").get<float>();
	m_cameraSettings.UpdateViewportSize();

	EntityData* maybeFollowTarget = TryDeserializeEntity(json.at("FollowTarget"), true);

	if (maybeFollowTarget != nullptr) SetFollowTarget(*maybeFollowTarget);
	else SetFollowNoTarget();
}
Json CameraComponent::Serialize()
{
	//TODO: we need to serailize the entity follow target
	Json json= { {"AspectRatio", m_cameraSettings.m_AspectRatio}, 
				 {"LensSize", m_cameraSettings.m_LensSize},
				 {"FollowTarget", TrySerializeEntity(m_cameraSettings.m_FollowTarget, true) }};

	return json;
}