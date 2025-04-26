#pragma once
#include <vector>
#include "TextBuffer.hpp"
#include "ColorGradient.hpp"
#include "RenderLayer.hpp"
#include "Point3D.hpp"
#include "ComponentData.hpp"

struct LightMapChar
{
	Vec2Int m_RelativePos;
	Utils::Point3D m_FractionalFilterColor;
	float m_ColorFactor;

	LightMapChar();
	LightMapChar(const Vec2Int& relativePos,
		const Utils::Point3D& fractionalFilterColor, const float& colorFactor);
	std::string ToString() const;
};

struct LightSourceData : public ComponentData
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

	//TODO: last frame data should instead be all positions/data of buffer added data
	//std::vector<TextCharArrayPosition> m_LastFrameData;
	std::vector<LightMapChar> m_LightMap;

	LightSourceData();
	LightSourceData(const Json& json);
	LightSourceData(const std::uint8_t& lightRadius, const RenderLayerType& affectedLayers, const ColorGradient& colorFilter,
		const std::uint8_t& intensity, const float& falloff);

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};