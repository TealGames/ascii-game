#pragma once
#include <vector>
#include "TextBuffer.hpp"
#include "ColorGradient.hpp"
#include "RenderLayer.hpp"

struct LightSourceData
{
	const std::uint8_t m_LightRadius;
	//The layers which the light will apply its effect to
	const RenderLayerType m_AffectedLayers;
	const ColorGradient m_GradientFilter;

	//The strength of the light initially
	const std::uint8_t m_Intensity;

	//where <1 creates more logarithmic curves, 
	//=1 creates linear and >1 creates exponential decay
	//and -1 means unused
	const float m_FalloffStrength;

	std::vector<TextCharPosition> m_LastFrameData;
	bool m_Dirty;

	LightSourceData();
	LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
		const std::uint8_t& intensity, const float& falloff);
};