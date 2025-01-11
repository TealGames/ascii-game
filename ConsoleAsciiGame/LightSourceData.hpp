#pragma once
#include <vector>
#include "TextBuffer.hpp"
#include "ColorGradient.hpp"
#include "RenderLayer.hpp"
#include "Point2D.hpp"
#include "Point3D.hpp"

struct LightMapChar
{
	Utils::Point2DInt m_RelativeCartesianPos;
	Utils::Point3D m_FractionalFilterColor;
	float m_ColorFactor;

	LightMapChar();
	LightMapChar(const Utils::Point2DInt& relativePos, 
		const Utils::Point3D& fractionalFilterColor, const float& colorFactor);
	std::string ToString() const;
};

struct LightSourceData
{
	std::uint8_t m_LightRadius;
	//The layers which the light will apply its effect to
	RenderLayerType m_AffectedLayers;
	ColorGradient m_GradientFilter;

	//The strength of the light initially
	std::uint8_t m_Intensity;

	//where <1 creates more logarithmic curves, 
	//=1 creates linear and >1 creates exponential decay
	//and -1 means unused
	float m_FalloffStrength;

	std::vector<TextCharPosition> m_LastFrameData;
	std::vector<LightMapChar> m_LightMap;

	LightSourceData();
	LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
		const std::uint8_t& intensity, const float& falloff);
};