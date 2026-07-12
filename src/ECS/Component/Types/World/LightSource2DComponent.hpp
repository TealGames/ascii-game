#pragma once
#include <vector>
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Primitives/ColorGradient.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/Color.hpp"

namespace Engine::Lighting2D
{
	struct LightMapChar
	{
		Vec2Int m_RelativePos;
		Vec3 m_FractionalFilterColor;
		float m_ColorFactor;

		LightMapChar();
		LightMapChar(const Vec2Int& relativePos,
			const Vec3& fractionalFilterColor, const float& colorFactor);
		std::string ToString() const;
	};

	struct LightSource2DComponent : public ECS::Component
	{
		std::uint8_t m_LightRadius;
		//The layers which the light will apply its effect to
		Rendering::RenderLayerType m_AffectedLayers;
		ColHDRGradient m_GradientFilter;
		ColHDR4 test;

		//The strength of the light initially
		std::uint8_t m_Intensity;

		//where <1 creates more logarithmic curves, 
		//=1 creates linear and >1 creates exponential decay
		//and -1 means unused
		float m_FalloffStrength;

		//TODO: last frame data should instead be all positions/data of buffer added data
		//std::vector<TextCharArrayPosition> m_LastFrameData;
		std::vector<LightMapChar> m_LightMap;

		LightSource2DComponent();
		LightSource2DComponent(const std::uint8_t& lightRadius, const Rendering::RenderLayerType& affectedLayers, const ColHDRGradient& colorFilter,
			const std::uint8_t& intensity, const float& falloff);

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}
