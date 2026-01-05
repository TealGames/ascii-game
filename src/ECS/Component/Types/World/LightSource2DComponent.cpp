//NOT USED
#include "pch.hpp"
#include "ECS/Component/Types/World/LightSource2DComponent.hpp"
#include "Core/Serialization/JsonSerializers.hpp"

LightMapChar::LightMapChar() : m_RelativePos(), m_ColorFactor() {}
LightMapChar::LightMapChar(const Vec2Int& relativePos,
	const Utils::Point3D& fractionalFilterColor, const float& colorFactor) :
	m_RelativePos(relativePos), m_FractionalFilterColor(fractionalFilterColor), m_ColorFactor(colorFactor) {}

std::string LightMapChar::ToString() const
{
	return std::format("[Rel:{}, FilterColor fraction: {} Factor:{}]", 
		m_RelativePos.ToString(), m_FractionalFilterColor.ToString(), std::to_string(m_ColorFactor));
}

LightSource2DComponent::LightSource2DComponent() : LightSource2DComponent(0, RenderLayerType::None, {}, 0, 0) {}

LightSource2DComponent::LightSource2DComponent(const Json& json) : LightSource2DComponent()
{
	Deserialize(json);
}
LightSource2DComponent::LightSource2DComponent(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
	const std::uint8_t& intensity, const float& falloff) :
	Component(), 
	m_LightRadius(lightRadius), m_GradientFilter(colorFilter), m_AffectedLayers(affectedLayers),
	m_Intensity(intensity), m_FalloffStrength(falloff), test()//, m_LastFrameData{}
{

}

void LightSource2DComponent::InitFields()
{
	m_Fields = {ComponentField("FalloffStrength", &m_FalloffStrength), 
		ComponentField("Radius", &m_LightRadius), ComponentField("Intensity", &m_Intensity), ComponentField("Color", &test)};
}
//std::vector<std::string> LightSourceData::GetDependencyFlags() const
//{
//	return {};
//}

std::string LightSource2DComponent::ToString() const
{
	return std::format("[LightSource Radius:{} Itensity:{} Falloff:{} Color:{} Layers:{}]", 
		std::to_string(m_LightRadius), std::to_string(m_Intensity), std::to_string(m_FalloffStrength), 
		m_GradientFilter.ToString(), ::ToString(m_AffectedLayers));
}

void LightSource2DComponent::Deserialize(const Json& json)
{
	m_LightRadius = json.at("Radius").get<std::uint8_t>();
	m_Intensity = json.at("Intensity").get<std::uint8_t>();
	m_FalloffStrength = json.at("Falloff").get<float>();
	m_GradientFilter = json.at("Color").get<ColorGradient>();
	m_AffectedLayers = json.at("Layers").get<RenderLayerType>();
}
Json LightSource2DComponent::Serialize()
{
	return { {"Radius", m_LightRadius}, {"Intensity", m_Intensity}, {"Falloff", m_FalloffStrength}, 
		{"Color", m_GradientFilter}, {"Layers", m_AffectedLayers}};
}