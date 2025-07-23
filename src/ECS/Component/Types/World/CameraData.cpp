#include "pch.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/PositionConversions.hpp"
#include "StaticGlobals.hpp"

CameraData::CameraData() : CameraData(CameraSettings()) {}

CameraData::CameraData(const Json& json) : CameraData()
{
	Deserialize(json);
}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	Component(), m_CameraSettings(cameraSettings), m_LastFrameBuffer() {}

ScreenPosition CameraData::WorldToScreenPosition(const WorldPosition& pos) const
{
	//TODO: could this maybe be precalculated to optimize
	const float topLeftY = GetEntity().GetTransform().GetGlobalPos().m_Y + (m_CameraSettings.m_WorldViewportSize.m_Y / 2);
	const float bottomLeftX = GetEntity().GetTransform().GetGlobalPos().m_X - (m_CameraSettings.m_WorldViewportSize.m_X / 2);
	Vec2 screenPercent{ (pos.m_X - bottomLeftX) / m_CameraSettings.m_WorldViewportSize.m_X,
				 (topLeftY - pos.m_Y) / m_CameraSettings.m_WorldViewportSize.m_Y };

	//TODO: right now we assume the position is for the whole screen, but we may not want the whole screen covered by the viewport
	return ScreenPosition{ screenPercent.m_X * SCREEN_WIDTH, screenPercent.m_Y * SCREEN_HEIGHT };
}
WorldPosition CameraData::ScreenToWorldPosition(const ScreenPosition& pos) const
{
	WorldPosition bottomLeftPos = GetEntity().GetTransform().GetGlobalPos() - (m_CameraSettings.m_WorldViewportSize / 2);
	Vec2 screenPercent{ pos.m_X / SCREEN_WIDTH, (SCREEN_HEIGHT - pos.m_Y) / SCREEN_HEIGHT };

	Vec2 worldSize = screenPercent * m_CameraSettings.m_WorldViewportSize;
	//LogWarning(std::format("CAMERA SHIT coord:{} screen percetn:{} viewport size:{}", pos.ToString(), screenPercent.ToString(), m_CameraSettings.m_WorldViewportSize.ToString()));
	return bottomLeftPos + worldSize;
}
Vec2 CameraData::WorldToScreenSize(const Vec2& worldSize) const
{
	return worldSize / m_CameraSettings.m_WorldViewportSize * Vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
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