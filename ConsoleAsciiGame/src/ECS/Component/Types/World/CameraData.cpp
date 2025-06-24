#include "pch.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

CameraData::CameraData() : CameraData(CameraSettings()) {}

CameraData::CameraData(const Json& json) : CameraData()
{
	Deserialize(json);
}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	Component(), m_CameraSettings(cameraSettings), m_LastFrameBuffer() {}

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