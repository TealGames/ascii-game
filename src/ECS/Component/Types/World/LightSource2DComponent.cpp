//NOT USED
#include "pch.hpp"
#include "ECS/Component/Types/World/LightSource2DComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Lighting2D
{
	LightMapChar::LightMapChar() : m_RelativePos(), m_ColorFactor() {}
	LightMapChar::LightMapChar(const Vec2Int& relativePos,
		const Vec3& fractionalFilterColor, const float& colorFactor) :
		m_RelativePos(relativePos), m_FractionalFilterColor(fractionalFilterColor), m_ColorFactor(colorFactor) {}

	std::string LightMapChar::ToString() const
	{
		return std::format("[Rel:{}, FilterColor fraction: {} Factor:{}]",
			m_RelativePos.ToString(), m_FractionalFilterColor.ToString(), std::to_string(m_ColorFactor));
	}

	LightSource2DComponent::LightSource2DComponent() : LightSource2DComponent(0, Rendering::RenderLayerType::None, {}, 0, 0) {}
	LightSource2DComponent::LightSource2DComponent(const std::uint8_t& lightRadius, 
		const Rendering::RenderLayerType& affectedLayers, const ColHDRGradient& colorFilter,
		const std::uint8_t& intensity, const float& falloff) :
		Component(),
		m_LightRadius(lightRadius), m_GradientFilter(colorFilter), m_AffectedLayers(affectedLayers),
		m_Intensity(intensity), m_FalloffStrength(falloff), test()//, m_LastFrameData{}
	{

	}

	void LightSource2DComponent::InitFields()
	{
		m_Fields = { ECS::ComponentField("FalloffStrength", &m_FalloffStrength),
			ECS::ComponentField("Radius", &m_LightRadius), ECS::ComponentField("Intensity", &m_Intensity), ECS::ComponentField("Color", &test) };
	}
	void LightSource2DComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("Radius", m_LightRadius);
		serializer.AddProperty("Intensity", m_Intensity);
		serializer.AddProperty("Falloff", m_FalloffStrength);
		serializer.AddProperty("Color", m_GradientFilter);
		serializer.AddProperty("Layers", m_AffectedLayers);
	}
	void LightSource2DComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("Radius", &m_LightRadius);
		deserializer.GetProperty("Intensity", &m_Intensity);
		deserializer.GetProperty("Falloff", &m_FalloffStrength);
		deserializer.GetProperty("Color", &m_GradientFilter);
		deserializer.GetProperty("Layers", &m_AffectedLayers);
	}

	std::string LightSource2DComponent::ToString() const
	{
		return std::format("[LightSource Radius:{} Itensity:{} Falloff:{} Color:{} Layers:{}]",
			std::to_string(m_LightRadius), std::to_string(m_Intensity), std::to_string(m_FalloffStrength),
			m_GradientFilter.ToString(), Rendering::ToString(m_AffectedLayers));
	}
}
