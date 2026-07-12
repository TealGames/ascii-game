#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/Color.hpp"

namespace Engine::Lighting3D
{
	constexpr float DEFAULT_LIGHT_INTENSITY = 1.0f;

	class PointLight3DComponent : public Engine::ECS::Component
	{
	private:
	public:
		ColHDR3 m_Color;
		/// <summary>
		/// The radius of the lighting influence sphere for this point light
		/// based in engine units
		/// </summary>
		float m_Radius;
		float m_Intensity;

	private:
	public:
		PointLight3DComponent();
		PointLight3DComponent(const ColHDR3 color, const float radius, const float intensity = DEFAULT_LIGHT_INTENSITY);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};

	/// <summary>
	/// When calculating light attenuation (light influence at a given distance)
	/// the minimum light contribution before there is no lighting effect
	/// </summary>
	constexpr float LIGHT_ATTENUATION_MIN_CONTRIBUTION = 0.01f;

	/// <summary>
	/// Calculates the approximate light radius given the lighting attenuation equation:
	/// 1 / (constant + linearTerm * d + quadraticTerm * d^2) where d is distance from light center
	/// </summary>
	/// <param name="constant"></param>
	/// <param name="linearTerm"></param>
	/// <param name="quadraticTerm"></param>
	/// <param name="minLightContribution"></param>
	/// <returns></returns>
	float InferLightRadiusFromAttenuation(float constant, float linearTerm, float quadraticTerm, 
		float minLightContribution = LIGHT_ATTENUATION_MIN_CONTRIBUTION);

	float InferLightIntensityFromAttenuation(float constant, float linearTerm, float quadraticTerm);
}
