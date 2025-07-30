#include "pch.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/PositionConversions.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Data/Vec4Type.hpp"
#include <cmath>

#if defined(OPENGL)
#include "Utils/OpenGlUtils.hpp"
#endif

CameraData::CameraData() : CameraData(CameraSettings()) {}

CameraData::CameraData(const Json& json) : CameraData()
{
	Deserialize(json);
}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	Component(), m_CameraSettings(cameraSettings), m_LastFrameBuffer() {}

void CameraData::SetNearDistance(const float near)
{
	if (near > m_CameraSettings.m_FarDistance)
	{
		LogWarning(std::format("Attempted to set camera:{} near distance:{} greater than far distance:{}", 
			GetEntity().ToString(), near, m_CameraSettings.m_FarDistance));
		return;
	}
	m_CameraSettings.m_NearDistance = std::abs(near);
}
void CameraData::SetFarDistance(const float far)
{
	if (far < m_CameraSettings.m_NearDistance)
	{
		LogWarning(std::format("Attempted to set camera:{} far distance:{} less than near distance:{}",
			GetEntity().ToString(), far, m_CameraSettings.m_NearDistance));
		return;
	}
	m_CameraSettings.m_FarDistance = std::abs(far);
}

float CameraData::GetNearDistance() const { return m_CameraSettings.m_NearDistance; }
float CameraData::GetFarDistance() const { return m_CameraSettings.m_FarDistance; }
float CameraData::GetViewDistance() const { return GetFarDistance() - GetNearDistance(); }

ScreenPosition CameraData::WorldToScreenPosition(const WorldPosition3D& pos) const
{
	//Note: we must use 4d vector for matrix multiplication -> 1 for w just means it is a point (0 would mean direction)
	const Vec4 clipPos = CalculateProjectionMatrix() * CalculateViewMatrix() * Vec4(pos, 1);
	Vec3 ndcPos = clipPos.GetXYZ() / clipPos.m_W;
}
Ray3D CameraData::ScreenToWorldPosition(const ScreenPosition& pos) const
{
	const WorldPosition3D cameraPos = GetEntity().GetTransform().GetGlobalPos();
	const WorldPosition2D bottomLeftPos = cameraPos.GetXY() - (m_CameraSettings.m_WorldViewportSize / 2);
	Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };

	Vec2 worldSize = screenPercent * m_CameraSettings.m_WorldViewportSize;
	//LogWarning(std::format("CAMERA SHIT coord:{} screen percetn:{} viewport size:{}", pos.ToString(), screenPercent.ToString(), m_CameraSettings.m_WorldViewportSize.ToString()));
	return Ray3D(WorldPosition3D(bottomLeftPos + worldSize, CalculateNearPlaneWorldCenter(cameraPos).m_Z), Vec3(0, 0, GetViewDistance()));
}
Vec2 CameraData::WorldToScreenSize(const float cameraDepthDistance, const Vec3& worldSize) const
{
	//if orthographic, since the size does not change as you move away, we just get fraction of world
	//object to viewport area within screen and we do not care about depth (Z axis) since it stays the same no matter the depth
	if (m_CameraSettings.m_ProjectionType== ProjectionType::Orthographic)	
		return worldSize.GetXY() / m_CameraSettings.m_WorldViewportSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	else
	{
		const Vec2 frustrumSize = m_CameraSettings.CalculateViewportSize(cameraDepthDistance);
		return worldSize.GetXY() / frustrumSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
}

Vec3 CameraData::CalculateWorldForward() const
{
	return GetTransform().GetGlobalRotation().ApplyRotationToDir(ENGINE_FORWARD_DIR);
}

WorldPosition3D CameraData::CalculateNearPlaneWorldCenter(const WorldPosition3D& globalPos) const
{
	return globalPos + (CalculateWorldForward() * m_CameraSettings.m_NearDistance);
}
WorldPosition3D CameraData::CalculateFarPlaneWorldCenter(const WorldPosition3D& globalPos) const
{
	return globalPos + (CalculateWorldForward() * m_CameraSettings.m_FarDistance);
}
WorldPosition3D CameraData::CalculateNearPlaneWorldCenter() const
{
	return CalculateNearPlaneWorldCenter(GetTransform().GetGlobalPos());
}
WorldPosition3D CameraData::CalculateFarPlaneWorldCenter() const
{
	return CalculateFarPlaneWorldCenter(GetTransform().GetGlobalPos());
}

std::array<InfinitePlane3D, 6> CameraData::CalculateFrustumPlanes() const
{
	const Mat4 viewProjection = CalculateProjectionMatrix() * CalculateViewMatrix();

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

bool CameraData::DoesViewVolumeContainPosOptimized(const WorldPosition3D& point, const std::array<InfinitePlane3D, 6>* precalculatedPlanes) const
{
	for (const auto& plane : *precalculatedPlanes)
	{
		//Note: the furstum planes have normals that point inward, so "forward" is considering inside frustum
		//and back of plane is considered outside view area
		if (plane.IsPointBackOfPlane(point))
			return false;
	}
	return true;
}
bool CameraData::DoesViewVolumeContainPos(const WorldPosition3D& point) const
{
	std::array<InfinitePlane3D, 6> frustumPlanes = CalculateFrustumPlanes();
	return DoesViewVolumeContainPosOptimized(point, &frustumPlanes);
}

Mat4 CameraData::CalculateViewMatrix() const
{
	//Since the rotation matrix is a special kind of matrix its inverse == tranpose (this is not normally true)
	const Mat4 invertedRotationMatrix = TransformData::CalculateRotationMatrix(GetTransform().GetGlobalRotation()).Transpose();
	const Vec4 rotatedTranslation = invertedRotationMatrix * Vec4(-GetTransform().GetGlobalPos(), 1.0f);
	return invertedRotationMatrix * TransformData::CalculateTranslationMatrix(rotatedTranslation.GetXYZ());
}

Mat4 CameraData::CalculateProjectionMatrix() const
{
	const float zNear = m_CameraSettings.m_NearDistance;
	const float zFar = m_CameraSettings.m_FarDistance;
	if (m_CameraSettings.m_ProjectionType == ProjectionType::Perspective)
	{
		const float aspect = m_CameraSettings.CalculateAspectRatio();
		const float fovY = m_CameraSettings.m_FieldOfViewYRadians;
		
		return Mat4(std::array<std::array<float, 4>, 4>
			{{
				{{ 1/(aspect *std::tan(fovY/2)),	0,						0,								0}},
				{{ 0,								1/(std::tan(fovY/2)),   0,								0}},
#if defined(OPENGL)
				//Since Opengl uses z forward->negative and -1 to 1 normalized z device coords
				//We have to compute the y and z parts of the matrix differendly depending on the framework used and their settings
				{{ 0,								0,						(zFar + zNear)/(zNear - zFar),	2*zFar*zNear / (zNear - zFar)}},
				{{ 0,								0,						-1,								0}}
#elif defined(DIRECTX)
				//DirectX uses z forward-> positive and [0,1] NDC
				{{ 0,								0,						zFar/(zFar-zNear),				-zNear*zFar / (zFar-zNear) }},
				{{ 0,								0,						1,								0}}
#else
#error "Unknown Graphics API: Failed to create perspective camera proj matrix"
				{{0, 0, 0, 0}},
				{{0, 0, 0, 0}}
#endif
			}});
	}
	else
	{
		const WorldPosition3D globalPos = GetTransform().GetGlobalPos();
		const float l = globalPos.m_X - m_CameraSettings.m_WorldViewportSize.m_X / 2;
		const float r = globalPos.m_X + m_CameraSettings.m_WorldViewportSize.m_X / 2;
		const float b = globalPos.m_Y - m_CameraSettings.m_WorldViewportSize.m_Y / 2;
		const float t = globalPos.m_Y + m_CameraSettings.m_WorldViewportSize.m_Y / 2;

		return Mat4(std::array<std::array<float, 4>, 4>
			{{
				{{ 2/ (r-l),	0,			0,					-(r+l)/(r-l)}},
				{{ 0,			2/(t-b),	0,					-(t+b)/(t-b)}},
#if defined(OPENGL)
				{{ 0,			0,			-2/(zFar-zNear),	-(zFar + zNear)/(zFar-zNear)}},
#elif defined(DIRECTX)
				{{ 0,			0,			1/(zFar-zNear),		-zNear/(zFar-zNear) }},
#else
#error "Unknown Graphics API: Failed to create orthographic camera proj matrix"
				{{ 0, 0, 0, 0}},
#endif			
				{{ 0,			0,			0,					1}}
			}});
	}
}

void CameraData::InitFields()
{
	m_Fields= { ComponentField("LensSize", &m_CameraSettings.m_LensSize) };
}
//std::vector<std::string> CameraData::GetDependencyFlags() const
//{
//	return {ENTITY_DEPENDENCY_FLAG};
//}

std::string CameraData::ToString() const
{
	return std::format("[Camera Settings:{}]", 
		m_CameraSettings.ToString());
}

void CameraData::Deserialize(const Json& json)
{
	m_CameraSettings.m_AspectRatio = json.at("AspectRatio").get<Vec2Int>();
	m_CameraSettings.m_LensSize = json.at("LensSize").get<float>();
	m_CameraSettings.UpdateViewportSize();

	EntityData* maybeFollowTarget = TryDeserializeEntity(json.at("FollowTarget"), true);

	if (maybeFollowTarget != nullptr) m_CameraSettings.SetFollowTarget(*maybeFollowTarget);
	else m_CameraSettings.SetFollowNoTarget();
}
Json CameraData::Serialize()
{
	//TODO: we need to serailize the entity follow target
	Json json= { {"AspectRatio", m_CameraSettings.m_AspectRatio}, 
				 {"LensSize", m_CameraSettings.m_LensSize},
				 {"FollowTarget", TrySerializeEntity(m_CameraSettings.m_FollowTarget, true) }};

	return json;
}