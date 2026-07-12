#include "ECS/Component/Types/World/PointLight3DComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Lighting3D
{
	PointLight3DComponent::PointLight3DComponent() : PointLight3DComponent({}, 0, 0) {}
	PointLight3DComponent::PointLight3DComponent(const ColHDR3 color, const float radius, const float intensity) :
		m_Radius(radius), m_Color(color), m_Intensity(intensity) {}

	void PointLight3DComponent::InitFields()
	{
		m_Fields = {};
	}
	void PointLight3DComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void PointLight3DComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string PointLight3DComponent::ToString() const
	{
		return std::format("[PointLight3D]");
	}

	float InferLightRadiusFromAttenuation(float c, float b, float a, float minLightContribution)
	{
		ENGINE_ASSERT(a >= 0.0f, "Quadratic attenuation term is negative which is not allowed");
		ENGINE_ASSERT(b >= 0.0f, "Linear attenuation term is negative which is not allowed");
		ENGINE_ASSERT(c >= 0.0f, "Constant attenuation term is negative which is not allowed");

		minLightContribution = std::max(minLightContribution, LIGHT_ATTENUATION_MIN_CONTRIBUTION);
		// Solve for d: 1 / (constant + linearTerm * d + quadraticTerm * d^2) = minLightContribution
		c = std::abs(c) - (1 / minLightContribution);

		if (::Math::ApproximateEqualsF(a, 0.0f))
		{
			if (::Math::ApproximateEqualsF(b, 0.0f))
				return 0.0f;

			return std::max(0.0f, -c / b);
		}

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			return 0.0f;

		return (-b + std::sqrt(discriminant)) / (2.0f * a);
	}

	float InferLightIntensityFromAttenuation(float constant, float linearTerm, float quadraticTerm)
	{
		return 1.0f / constant;
	}
}
