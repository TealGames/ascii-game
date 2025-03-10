#include "pch.hpp"

#include "CameraData.hpp"
#include "CameraSettings.hpp"
#include "TextBuffer.hpp"
#include "ComponentData.hpp"
#include "JsonSerializers.hpp"
#include "Entity.hpp"

CameraData::CameraData() :
	ComponentData(), m_CameraSettings(), m_LastFrameBuffer(){}

CameraData::CameraData(const Json& json) : CameraData()
{
	Deserialize(json);
}

CameraData::CameraData(const CameraSettings& cameraSettings) :
	ComponentData(), m_CameraSettings(cameraSettings), m_LastFrameBuffer() {}

void CameraData::InitFields()
{
	m_Fields= { ComponentField("LensSize", &m_CameraSettings.m_LensSize) };
}

std::string CameraData::ToString() const
{
	return std::format("[Camera Settings:{}]", 
		m_CameraSettings.ToString());
}

CameraData& CameraData::Deserialize(const Json& json)
{
	m_CameraSettings.m_AspectRatio = json.at("AspectRatio").get<Vec2Int>();
	m_CameraSettings.m_LensSize = json.at("LensSize").get<float>();
	m_CameraSettings.UpdateViewportSize();

	//TODO: there is not really any way to get the follow target if we do not know 
	//all the entities considering we can only store string name
	m_CameraSettings.SetFollowNoTarget();
	return *this;
}
Json CameraData::Serialize(const CameraData& component)
{
	return { {"AspectRatio", m_CameraSettings.m_AspectRatio}, 
			 {"LensSize", m_CameraSettings.m_LensSize},
			 {"FollowTarget", m_CameraSettings.m_FollowTarget!=nullptr? m_CameraSettings.m_FollowTarget->m_Name : ""}};
}