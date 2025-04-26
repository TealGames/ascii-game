//NOT USED
#include "pch.hpp"

#include "LightSourceData.hpp"
#include "JsonSerializers.hpp"

LightMapChar::LightMapChar() : m_RelativePos(), m_ColorFactor() {}
LightMapChar::LightMapChar(const Vec2Int& relativePos,
	const Utils::Point3D& fractionalFilterColor, const float& colorFactor) :
	m_RelativePos(relativePos), m_FractionalFilterColor(fractionalFilterColor), m_ColorFactor(colorFactor) {}

std::string LightMapChar::ToString() const
{
	return std::format("[Rel:{}, FilterColor fraction: {} Factor:{}]", 
		m_RelativePos.ToString(), m_FractionalFilterColor.ToString(), std::to_string(m_ColorFactor));
}

LightSourceData::LightSourceData() : LightSourceData(0, RenderLayerType::None, {}, 0, 0) {}

LightSourceData::LightSourceData(const Json& json) : LightSourceData()
{
	Deserialize(json);
}
LightSourceData::LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
	const std::uint8_t& intensity, const float& falloff) :
	ComponentData(), 
	m_LightRadius(lightRadius), m_GradientFilter(colorFilter), m_AffectedLayers(affectedLayers),
	m_Intensity(intensity), m_FalloffStrength(falloff)//, m_LastFrameData{}
{

}

void LightSourceData::InitFields()
{
	m_Fields = {ComponentField("FalloffStrength", &m_FalloffStrength), 
		ComponentField("Radius", &m_LightRadius), ComponentField("Intensity", &m_Intensity)};
}
std::vector<std::string> LightSourceData::GetDependencyFlags() const
{
	return {};
}

std::string LightSourceData::ToString() const
{
	return std::format("[LightSource Radius:{} Itensity:{} Falloff:{} Color:{} Layers:{}]", 
		std::to_string(m_LightRadius), std::to_string(m_Intensity), std::to_string(m_FalloffStrength), 
		m_GradientFilter.ToString(), ::ToString(m_AffectedLayers));
}

void LightSourceData::Deserialize(const Json& json)
{
	m_LightRadius = json.at("Radius").get<std::uint8_t>();
	m_Intensity = json.at("Intensity").get<std::uint8_t>();
	m_FalloffStrength = json.at("Falloff").get<float>();
	m_GradientFilter = json.at("Color").get<ColorGradient>();
	m_AffectedLayers = json.at("Layers").get<RenderLayerType>();
}
Json LightSourceData::Serialize()
{
	return { {"Radius", m_LightRadius}, {"Intensity", m_Intensity}, {"Falloff", m_FalloffStrength}, 
		{"Color", m_GradientFilter}, {"Layers", m_AffectedLayers}};
}