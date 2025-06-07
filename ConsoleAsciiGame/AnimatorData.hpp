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
#include "HelperFunctions.hpp"
#include "ComponentData.hpp"
#include "ComponentFieldReference.hpp"
#include "Debug.hpp"

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
		if (!Assert(this, m_value != std::nullopt, 
			std::format("Tried to get value from animation property key frame but value is NULL")))
			throw std::invalid_argument("Invalid value state");

		return m_value.value();
	}

	std::string ToString() const
	{
		return std::format("[Time: {} Value: {}]", 
			std::to_string(m_time), Utils::TryToString<T>(GetValue()).value_or(
				std::format("PARSE FAILED: {}", Utils::GetTypeName<T>())));
	}
};

template<typename T>
struct AnimationProperty
{
	std::vector<AnimationPropertyKeyframe<T>> m_Keyframes;
	size_t m_KeyframeIndex;
	ComponentFieldReference m_ComponentFieldRef;
	//T& m_ComponentPropertyRef;
	//bool& m_ComponentDataMutationFlagRef;


	/*AnimationProperty(T& compoenentPropertyRef, bool& componentDataMutationRef, const std::vector<AnimationPropertyKeyframe<T>> keyframes) :
		m_ComponentPropertyRef(compoenentPropertyRef), m_ComponentDataMutationFlagRef(componentDataMutationRef), m_Keyframes(keyframes), m_KeyframeIndex(0) {}*/

	AnimationProperty() : AnimationProperty(ComponentFieldReference(), {}) {}

	AnimationProperty(const ComponentFieldReference& fieldRef, const std::vector<AnimationPropertyKeyframe<T>>& keyframes) :
		m_ComponentFieldRef(fieldRef), m_Keyframes(keyframes), m_KeyframeIndex(0) 
	{
		//TODO: make sure the field reference type is the same as the t type
	}

	bool TrySetValue(T value)
	{
		ComponentField& field = m_ComponentFieldRef.GetComponentFieldSafeMutable();
		return field.TrySetValue<T>(value);
	}

	std::string ToString() const
	{
		return std::format("[Prop:{} keyframes:{}]", typeid(T).name(), 
			Utils::ToStringIterable<std::vector<AnimationPropertyKeyframe<T>>, AnimationPropertyKeyframe<T>>(m_Keyframes));
	}

	bool operator==(const AnimationProperty& other) const = default;
};

//template <typename... Types>
//using GeneratedVariant = std::variant<AnimationProperty<Types>...>;
//
//using AnimationPropertyVariant = GeneratedVariant<int, float, std::uint8_t>;
using AnimationPropertyVariant = std::variant<AnimationProperty<int>, AnimationProperty<float>, AnimationProperty<std::uint8_t>>;
class AnimatorData : public Component
{
private:
	bool m_Loop;
	
	float m_AnimationSpeed;
	/// <summary>
	/// The time in takes in seconds on speed of 1 to finish one loop of the animation
	/// </summary>
	float m_AnimationLength;

public:
	//All properties for the animator for this entity. 
	//Note: this is exposed because we need to acess properties using functions woth no restrictions
	std::vector<AnimationPropertyVariant> m_Properties;
	float m_NormalizedTime;
	size_t m_KeyframeIndex;

private:
public:
	AnimatorData();
	AnimatorData(const Json& json);
	AnimatorData(const std::vector<AnimationPropertyVariant>& properties,
		const float& animationTime, const float& speed, const bool& loop);

	const bool& GetDoLoop() const;
	const std::vector<AnimationPropertyVariant>& GetProperties() const;
	const float& GetAnimationSpeed() const;
	void SetAnimationSpeed(const float& speed);

	const float& GetTimeLength() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

template<typename T>
struct AnimationPropertyType;

template<typename T>
struct AnimationPropertyType<AnimationProperty<T>>
{
	using Type = T;
};

