#include "pch.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/PositionConversions.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Data/Quaternion.hpp"
#include "Utils/Data/Vec4Type.hpp"
#include "Math/PlatformMath.hpp"

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

CameraComponent::CameraComponent(const Json& json) : CameraComponent()
{
	Deserialize(json);
}

CameraComponent::CameraComponent(const CameraSettings& cameraSettings) :
	Component(), m_cameraSettings(cameraSettings), m_lastUpdateData() 
{
	m_isDirty = true;
}

bool CameraComponent::IsDirty() const
{
	return m_isDirty || GetTransform().IsDirty();
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

	if (Utils::ApproximateEqualsF(clipPos.m_W, 0))
		return INVALID_NDC_POS;
	return clipPos.GetXYZ() / clipPos.m_W;
	
}

ScreenPosition CameraComponent::WorldToScreenPosition(const WorldPosition3D& pos) const
{
	//Note: we must use 4d vector for matrix multiplication -> 1 for w just means it is a point (0 would mean direction)
	/*const Vec4 posViewMat = m_lastUpdateData.m_EngineProjectionMatrix * Vec4(0, 0, 0, 1);
	LogError(std::format("engine:{} default proj:{} new math:{}", 
		m_lastUpdateData.m_EngineProjectionMatrix.ToString(), 
		m_lastUpdateData.m_PlatformProjectionMatrix.ToString(), posViewMat.ToString()));*/
	/*
	float A[4][4] = {
		{1,  2,  3,  4} ,
  {5,  6,  7,  8},
  {9, 10, 11, 12},
  {13,14, 15, 16}
	};
	const Mat4 a{ A };
	const Vec4 b(1, 2, 3, 4);
	LogError(std::format("A*b={}", (a * b).ToString()));

	float B[4][4] = {
		{17, 18, 19, 20},
		{21, 22, 23, 24},
		{25, 26, 27, 28},
		{29, 30, 31, 32}
	};
	*/
	//const Mat4 b{ B };
	//LogError(std::format("A*b={} \nB*A={}", (a*b).ToString(), (b*a).ToString()));
	
	//LogError(std::format("projViewmat:{}", projViewMat.ToString()));
	const Vec4 clipPos = GetLastUpdateData().m_EngineProjectionMatrix * GetLastUpdateData().m_ViewMatrix * Vec4(pos, 1);
	/*LogError(std::format("Got clip pos:{} ViewMatrix:{} engineProjMatrix:{} pos:{}", clipPos.ToString(), 
		m_lastUpdateData.m_ViewMatrix.ToString(), m_lastUpdateData.m_EngineProjectionMatrix.ToString(), Vec4(pos,1).ToString()));*/

	//Mat4 projViewMat = m_lastUpdateData.m_EngineProjectionMatrix * m_lastUpdateData.m_ViewMatrix;
	/*LogError(std::format("\nCamera transfrom:{} \ncamera viewportsize:{} \nSettings:{}\nPreclac data:{}\nproj:{} \nview:{} \npos:{} (IN VIEW:{}) \nproj*view={} "
		"\nprojView*pos={} clip pos:{}",
		GetTransform().ToString(), m_cameraSettings.CalculateViewportSize(m_cameraSettings.m_NearDistance).ToString(),
		m_cameraSettings.ToString(), m_lastUpdateData.ToString(),
		m_lastUpdateData.m_EngineProjectionMatrix.ToString(),
		m_lastUpdateData.m_ViewMatrix.ToString(), Vec4(pos, 1).ToString(), DoesViewVolumeContainPos(pos),
		projViewMat.ToString(), (projViewMat * Vec4(pos, 1)).ToString(), clipPos.ToString()));*/

	//This means we are outside view and cannot flatten 4d pos -> 3d
	if (Utils::ApproximateEqualsF(clipPos.m_W, 0))
	{
		LogError(std::format("Attempted to convert world pos:{} (clip pos:{}) to screen using camera:{}"
			"but that world pos is outside the camera view and thus cannot be mapped to a valid screen position. "
			"View area at its z depth:{}", pos.ToString(), clipPos.ToString(), GetTransform().ToString(),
			m_cameraSettings.CalculateViewportSize(pos.m_Z- m_cameraSettings.m_NearDistance).ToString()));
		return {};
	}

	Vec3 ndcPos = clipPos.GetXYZ() / clipPos.m_W;
	for (size_t i=0; i<3; i++)
	{
		if (ndcPos[i] > 1 || (ENGINE_NDC_RANGES[i] == NdcRange::NegOneToOne && ndcPos[i] < -1) ||
			(ENGINE_NDC_RANGES[i] == NdcRange::ZeroToOne && ndcPos[i] < 0))
		{
			LogError(std::format("Attempted to convert world pos:{} (ndc pos:{}) to screen using camera:{}"
				"but that world pos is outside the camera view and thus cannot be mapped to a valid screen position. "
				"View Area at itsz depth:{}", pos.ToString(), ndcPos.ToString(), GetTransform().ToString(), 
				m_cameraSettings.CalculateViewportSize(pos.m_Z- m_cameraSettings.m_NearDistance).ToString()));
			return {};
		}
	}

	//This ensures that we have x and y normalized from 0 to 1
	if (ENGINE_NDC_RANGES[0] == NdcRange::NegOneToOne) ndcPos.m_X = (ndcPos.m_X + 1) * 0.5;
	if (ENGINE_NDC_RANGES[1] == NdcRange::NegOneToOne) ndcPos.m_Y = (ndcPos.m_Y + 1) * 0.5;
	//LogError(std::format("ndc pos:{}", ndcPos.ToString()));
	//Note: since we use top left as origin for screen pos (and y increases down), we have to invert y pos
	return ScreenPosition(ndcPos.m_X * SCREEN_WIDTH, (1- ndcPos.m_Y)*SCREEN_HEIGHT);
}
Ray3D CameraComponent::ScreenToWorldPosition(const ScreenPosition& pos) const
{
	const WorldPosition3D cameraPos = GetEntity().GetTransform().GetGlobalPos();
	const Vec2 nearSize = m_cameraSettings.CalculateViewportSize(m_cameraSettings.m_NearDistance);
	//const WorldPosition2D bottomLeftPos = cameraPos.GetXY() - (m_cameraSettings.m_WorldViewportSize / 2);
	const Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };
	const float camX = (screenPercent.m_X - 0.5f) * nearSize.m_X;
	const float camY = (0.5f- screenPercent.m_Y) * nearSize.m_Y;

	const Vec3 nearPoint = cameraPos + CalculateWorldForward() * m_cameraSettings.m_NearDistance + 
		CalculateWorldRight() * camX + CalculateWorldUp() * camY;

	Vec3 dir = {};
	WorldPosition3D origin = {};
	if (m_cameraSettings.m_ProjectionType == ProjectionType::Orthographic)
	{
		origin = nearPoint;
		dir = CalculateWorldForward();
	}
	else
	{
		origin = cameraPos;
		dir = (nearPoint - cameraPos).GetNormalized();
	}
	//const Vec3 dir = GetTransform().GetGlobalRotation().ApplyRotationToDir(Vec3(0, 0, 1)) * ;
	//LogWarning(std::format("CAMERA SHIT coord:{} screen percetn:{} viewport size:{}", pos.ToString(), screenPercent.ToString(), m_CameraSettings.m_WorldViewportSize.ToString()));
	return Ray3D(origin, dir * GetViewDistance());
}
Vec2 CameraComponent::WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const
{
	//if orthographic, since the size does not change as you move away, we just get fraction of world
	//object to viewport area within screen and we do not care about depth (Z axis) since it stays the same no matter the depth
	if (m_cameraSettings.m_ProjectionType== ProjectionType::Orthographic)	
		return worldSize.GetXY() / m_cameraSettings.CalculateViewportSize() * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
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
Vec3 CameraComponent::CalculateWorldUp() const
{
	return GetTransform().GetGlobalRotation().ApplyRotationToDir(ENGINE_UP_DIR);
}
Vec3 CameraComponent::CalculateWorldRight() const
{
	return CrossProduct(CalculateWorldUp(), CalculateWorldForward()).GetNormalized();
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
	//Note: here we must use data directly not via function to prevent recursive loop + 
	//since we calculate this last, we are guaranteed to have the most updated proj and view matrix
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
	for (const auto& plane : GetLastUpdateData().m_FrustumPlanes)
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
	const Mat4 invertedRotationMatrix = CalculateRotationMatrix(GetTransform().GetGlobalRotation()).Transpose();
	const Vec4 rotatedTranslation = invertedRotationMatrix * Vec4(-GetTransform().GetGlobalPos(), 1.0f);
	return invertedRotationMatrix * CalculateTranslationMatrix(rotatedTranslation.GetXYZ());
}

Mat4 CameraComponent::CalculateProjectionMatrix(const ProjectionMatrixType type) const
{
	const float zNear = m_cameraSettings.m_NearDistance;
	const float zFar = m_cameraSettings.m_FarDistance;
	if (m_cameraSettings.m_ProjectionType == ProjectionType::Perspective)
	{
		if (type == ProjectionMatrixType::Engine)
		{
			return PlatformMath::CalculatePerspectiveProjMatrix(ENGINE_NDC_RANGES[2], ENGINE_FORWARD_SIGN_Z, m_cameraSettings.m_FieldOfViewYRadians,
				m_cameraSettings.CalculateAspectRatio(), zNear, zFar);
		}

		return PlatformMath::CalculatePlatformPerspectiveProjMatrix(m_cameraSettings.m_FieldOfViewYRadians, 
			m_cameraSettings.CalculateAspectRatio(), zNear, zFar);
	}
	else
	{
		const WorldPosition3D globalPos = GetTransform().GetGlobalPos();
		const Vec2 viewportSize = m_cameraSettings.CalculateViewportSize();
		const float l = globalPos.m_X - viewportSize.m_X / 2;
		const float r = globalPos.m_X + viewportSize.m_X / 2;
		const float b = globalPos.m_Y - viewportSize.m_Y / 2;
		const float t = globalPos.m_Y + viewportSize.m_Y / 2;

		if (type == ProjectionMatrixType::Engine)
			PlatformMath::CalculateOrthographicProjMatrix(ENGINE_NDC_RANGES[2], ENGINE_FORWARD_SIGN_Z, r, l, t, b, zNear, zFar);

		return PlatformMath::CalculatePlatformOrthographicProjMatrix(r, l, t, b, zNear, zFar);
	}
}

/*
void CameraComponent::UpdatePrecalculatedData()
{
	//TODO: do we need platform or engine frustum planes?
	m_lastUpdateData.m_ViewMatrix = CalculateViewMatrix();
	m_lastUpdateData.m_PlatformProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Platform);
	m_lastUpdateData.m_EngineProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Engine);
	//LogError(std::format("view mat:{} pproj:{} eproj:{}", m_lastUpdateData.m_ViewMatrix.ToString(), 
	//m_lastUpdateData.m_PlatformProjectionMatrix.ToString(), m_lastUpdateData.m_EngineProjectionMatrix.ToString()));
	m_lastUpdateData.m_FrustumPlanes = CalculateFrustumPlanes();
}
*/
const CameraPrecalculatedData& CameraComponent::GetLastUpdateData() const 
{ 
	const bool cameraMoved = GetTransform().IsDirty();
	const bool cameraSettingsUpdated = m_isDirty;
	if (cameraMoved)
	{
		m_lastUpdateData.m_ViewMatrix = CalculateViewMatrix();
		m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::ViewMatrix;
	}
	if (cameraSettingsUpdated)
	{
		m_lastUpdateData.m_PlatformProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Platform);
		m_lastUpdateData.m_EngineProjectionMatrix = CalculateProjectionMatrix(ProjectionMatrixType::Engine);
		m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::EngineProjMatrix | CameraPrecalculatedDataUpdate::PlatformProjMatrix;
		m_isDirty = false;
	}
	if (cameraMoved || cameraSettingsUpdated)
	{
		m_lastUpdateData.m_FrustumPlanes = CalculateFrustumPlanes();
		m_lastUpdateData.m_UpdatedThisFrame |= CameraPrecalculatedDataUpdate::FrustumPlanes;
	}
	return m_lastUpdateData; 
}

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

	EntityData* maybeFollowTarget = TryDeserializeEntity(json.at("FollowTarget"), true);

	if (maybeFollowTarget != nullptr) SetFollowTarget(*maybeFollowTarget);
	else SetFollowNoTarget();

	m_isDirty = true;
}
Json CameraComponent::Serialize()
{
	//TODO: we need to serailize the entity follow target
	Json json= { {"AspectRatio", m_cameraSettings.m_AspectRatio}, 
				 {"LensSize", m_cameraSettings.m_LensSize},
				 {"FollowTarget", TrySerializeEntity(m_cameraSettings.m_FollowTarget, true) }};

	return json;
}