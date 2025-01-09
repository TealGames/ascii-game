//NOT USED
#include "pch.hpp"

#include "LightSourceData.hpp"

LightSourceData::LightSourceData() : LightSourceData(0, RenderLayerType::None, {}, 0, 0) {}

LightSourceData::LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
	const std::uint8_t& intensity, const float& falloff) :
	m_LightRadius(lightRadius), m_GradientFilter(colorFilter), m_AffectedLayers(affectedLayers),
	m_Intensity(intensity), m_FalloffStrength(falloff), m_LastFrameData{}
{

}