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

ScreenPosition CameraData::WorldToScreenPosition(const WorldPosition3D& pos) const
{
	//TODO: could this maybe be precalculated to optimize
	const float topLeftY = GetEntity().GetTransform().GetGlobalPos().m_Y + (m_CameraSettings.m_WorldViewportSize.m_Y / 2);
	const float bottomLeftX = GetEntity().GetTransform().GetGlobalPos().m_X - (m_CameraSettings.m_WorldViewportSize.m_X / 2);
	Vec2 screenPercent{ (pos.m_X - bottomLeftX) / m_CameraSettings.m_WorldViewportSize.m_X,
				 (topLeftY - pos.m_Y) / m_CameraSettings.m_WorldViewportSize.m_Y };

	//TODO: right now we assume the position is for the whole screen, but we may not want the whole screen covered by the viewport
	return ScreenPosition{ screenPercent.m_X * SCREEN_WIDTH, screenPercent.m_Y * SCREEN_HEIGHT };
}
WorldPosition3D CameraData::ScreenToWorldPosition(const ScreenPosition& pos) const
{
	WorldPosition3D bottomLeftPos = GetEntity().GetTransform().GetGlobalPos() - (m_CameraSettings.m_WorldViewportSize / 2);
	Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };

	Vec2 worldSize = screenPercent * m_CameraSettings.m_WorldViewportSize;
	//LogWarning(std::format("CAMERA SHIT coord:{} screen percetn:{} viewport size:{}", pos.ToString(), screenPercent.ToString(), m_CameraSettings.m_WorldViewportSize.ToString()));
	return bottomLeftPos + worldSize;
}
Vec2 CameraData::WorldToScreenSize(const Vec2& worldSize) const
{
	//if orthographic, since the size does not change as you move away, we just get fraction of world
	//object to viewport area within screen
	if (m_CameraSettings.m_ProjectionType== ProjectionType::Orthographic)	
		return worldSize / m_CameraSettings.m_WorldViewportSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
}

Vec3 CameraData::GetLocalForward() const
{
#if defined(OPENGL)
	return Vec3(0, 0, -1);
#else
	LogError(std::format("Attempted to get local forward for camera for current framework "
		"(or none were selected) but there were none defined"));
	return {};
#endif
}
Vec3 CameraData::CalculateWorldForward() const
{
	return GetTransform().GetGlobalRotation().ApplyRotationToDir(GetLocalForward());
}

WorldPosition3D CameraData::CalculateNearPlaneWorldCenter() const
{
	return GetTransform().GetGlobalPos() + (CalculateWorldForward() * m_CameraSettings.m_NearDistance);
}
WorldPosition3D CameraData::CalculateFarPlaneWorldCenter() const
{
	return GetTransform().GetGlobalPos() + (CalculateWorldForward() * m_CameraSettings.m_FarDistance);
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
	if (m_CameraSettings.m_ProjectionType == ProjectionType::Perspective)
	{
		const float aspect = m_CameraSettings.CalculateAspectRatio();
		const float fovY = m_CameraSettings.m_FieldOfViewYRadians;
		const float zNear = m_CameraSettings.m_NearDistance;
		const float zFar = m_CameraSettings.m_FarDistance;

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
				{{ 0,								0,						zFar/(zFar-zNear),				-zNear*zFar / (zFar-zNear) }},
				{{ 0,								0,						1,								0}}
#else
#error "Failed to create camera proj matrix"
				{{0, 0, 0, 0}},
				{{0, 0, 0, 0}}
#endif
			}});
	}
	else
	{

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