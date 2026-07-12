#pragma once
#include <format>
#include <vector>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <string>
#include <variant>
#include <optional>
#include <type_traits>
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Component/ComponentFieldReference.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Animation
{
	template<typename T>
	class AnimationPropertyKeyframe
	{
	private:
		float m_time;
		std::optional<T> m_value;
	public:

	private:
	public:
		AnimationPropertyKeyframe() : m_value(std::nullopt), m_time(0) {}
		AnimationPropertyKeyframe(const T& value, const float& time) :
			m_value(value), m_time(time) {}

		const float& GetTime() const { return m_time; }
		const T& GetValue() const
		{
			ENGINE_ASSERT(m_value != std::nullopt,
				"Tried to get value from animation property key frame but value is NULL");

			return m_value.value();
		}

		std::string ToString() const
		{
			return std::format("[Time: {} Value: {}]",
				std::to_string(m_time), ::Utils::TryToString<T>(GetValue()).value_or(
					std::format("PARSE FAILED: {}", ::Utils::ToStringTypeName<T>())));
		}
	};

	template<typename T>
	struct AnimationProperty
	{
		std::vector<AnimationPropertyKeyframe<T>> m_Keyframes;
		size_t m_KeyframeIndex;
		ECS::ComponentFieldReference m_ComponentFieldRef;

		AnimationProperty() : AnimationProperty(ECS::ComponentFieldReference(), {}) {}

		AnimationProperty(const ECS::ComponentFieldReference& fieldRef, const std::vector<AnimationPropertyKeyframe<T>>& keyframes) :
			m_ComponentFieldRef(fieldRef), m_Keyframes(keyframes), m_KeyframeIndex(0)
		{
			//TODO: make sure the field reference type is the same as the t type
		}

		bool TrySetValue(T value)
		{
			ECS::ComponentField& field = m_ComponentFieldRef.GetComponentFieldSafeMutable();
			return field.TrySetValue<T>(value);
		}

		std::string ToString() const
		{
			return std::format("[Prop:{} keyframes:{}]", typeid(T).name(),
				::Utils::ToStringIterable<std::vector<AnimationPropertyKeyframe<T>>>(m_Keyframes));
		}

		bool operator==(const AnimationProperty& other) const = default;
	};

	using AnimationPropertyVariant = std::variant<AnimationProperty<int>, AnimationProperty<float>, AnimationProperty<std::uint8_t>>;
	class AnimatorComponent : public ECS::Component
	{
	private:
		bool m_loop;
		float m_animationSpeed;
		/// <summary>
		/// The time in seconds for 1 animation loop on 1.0 speed
		/// </summary>
		float m_animationLength;

	public:
		size_t m_KeyframeIndex;
		float m_NormalizedTime;
		//All properties for the animator for this entity. 
		//Note: this is exposed because we need to acess properties using functions woth no restrictions
		std::vector<AnimationPropertyVariant> m_Properties;

	private:
		float SetAnimationLength(float time);
	public:
		AnimatorComponent();
		AnimatorComponent(const std::vector<AnimationPropertyVariant>& properties,
			const float& animationTime, const float& speed, const bool& loop);

		bool GetDoLoop() const;
		const std::vector<AnimationPropertyVariant>& GetProperties() const;
		float GetAnimationSpeed() const;
		/// <summary>
		/// The multiplier on the animation time duration where 1.0 is default rate,
		/// 0.5 is double the time (so a normal 1 sec animation would be 2 sec), 2.0 is half time, etc.
		/// </summary>
		/// <param name="speed">>=0 animation time multiplier</param>
		/// <returns>the value set for the speed</returns>
		float SetAnimationSpeed(float speed);
		
		/// <summary>
		/// Will return the time length in seconds of 1 animation loop on default speed (1.0)
		/// </summary>
		/// <returns></returns>
		float GetDefaultTimeLength() const;
		/// <summary>
		/// Will return the time length in seconds of 1 animation loop based on animation speed
		/// </summary>
		/// <returns></returns>
		float GetRealTimeLength() const;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};

	template<typename T>
	struct AnimationPropertyType;

	template<typename T>
	struct AnimationPropertyType<AnimationProperty<T>>
	{
		using Type = T;
	};
}