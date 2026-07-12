#include "pch.hpp"
#include "ECS/Component/Types/World/AnimatorComponent.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/Debug.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Animation
{
	AnimatorComponent::AnimatorComponent() : AnimatorComponent({}, 0, 1, false) {}

	AnimatorComponent::AnimatorComponent(const std::vector<AnimationPropertyVariant>& properties, const float& animationTime, const float& speed, const bool& loop) :
		Component(),
		m_Properties(properties), m_animationSpeed(0.0f), m_NormalizedTime(0.0f), m_KeyframeIndex(0), m_animationLength(0.0f), m_loop(loop)
	{
		SetAnimationSpeed(speed);
		SetAnimationLength(animationTime);
	}

	bool AnimatorComponent::GetDoLoop() const { return m_loop; }

	const std::vector<AnimationPropertyVariant>& AnimatorComponent::GetProperties() const
	{
		return m_Properties;
	}

	float AnimatorComponent::GetAnimationSpeed() const { return m_animationSpeed; }
	float AnimatorComponent::SetAnimationSpeed(float speed)
	{
		m_animationSpeed = std::abs(speed);
		return m_animationSpeed;
	}

	float AnimatorComponent::GetDefaultTimeLength() const { return m_animationLength; }
	float AnimatorComponent::GetRealTimeLength() const { return m_animationLength * m_animationSpeed; }
	float AnimatorComponent::SetAnimationLength(float time)
	{
		m_animationLength = std::abs(time);
		return m_animationLength;
	}

	void AnimatorComponent::InitFields()
	{
		m_Fields = { ECS::ComponentField("Loop", &m_loop), ECS::ComponentField("Speed",
			(std::function<void(float)>)[this](float speed)->void {SetAnimationSpeed(speed); }, &m_animationSpeed) };
	}

	void AnimatorComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("Loop", m_loop);
		serializer.AddProperty("Speed", m_animationSpeed);
		serializer.AddProperty("Time", m_animationLength);
		serializer.AddProperty("Properties", m_Properties);
	}
	void AnimatorComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("Loop", &m_loop);

		float animSpeed = 0.0f;
		deserializer.GetProperty("Speed", &animSpeed);
		SetAnimationSpeed(animSpeed);

		float animTime = 0.0f;
		deserializer.GetProperty("Time", &animTime);
		SetAnimationLength(animTime);

		deserializer.GetProperty("Properties", &m_Properties);
	}

	std::string AnimatorComponent::ToString() const
	{
		std::vector<std::string> propertyStrs = {};
		for (auto& property : m_Properties)
		{
			std::visit([&property, &propertyStrs](auto&& value) -> void
				{
					using PropertyConvertedType = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
					using ExtractedType = AnimationPropertyType<PropertyConvertedType>::Type;

					propertyStrs.emplace_back(std::get<PropertyConvertedType>(property).ToString());

				}, property);

		}
		return std::format("[Animator Loop:{} Speed:{} Properties:{}]",
			std::to_string(m_loop), std::to_string(m_animationSpeed),
			::Utils::ToStringIterable(propertyStrs));
	}
}