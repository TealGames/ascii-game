//NOT USED
#include "pch.hpp"

#include "LightSourceData.hpp"

LightMapChar::LightMapChar() : m_RelativePos(), m_ColorFactor() {}
LightMapChar::LightMapChar(const CartesianPosition& relativePos,
	const Utils::Point3D& fractionalFilterColor, const float& colorFactor) :
	m_RelativePos(relativePos), m_FractionalFilterColor(fractionalFilterColor), m_ColorFactor(colorFactor) {}

std::string LightMapChar::ToString() const
{
	return std::format("[Rel:{}, FilterColor fraction: {} Factor:{}]", 
		m_RelativePos.ToString(), m_FractionalFilterColor.ToString(), std::to_string(m_ColorFactor));
}

LightSourceData::LightSourceData() : LightSourceData(0, RenderLayerType::None, {}, 0, 0) {}

LightSourceData::LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
	const std::uint8_t& intensity, const float& falloff) :
	ComponentData(), m_LightRadius(lightRadius), m_GradientFilter(colorFilter), m_AffectedLayers(affectedLayers),
	m_Intensity(intensity), m_FalloffStrength(falloff), m_LastFrameData{}
{

}