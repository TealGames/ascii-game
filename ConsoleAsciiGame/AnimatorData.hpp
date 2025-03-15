#pragma once
#include <format>
#include <vector>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <string>
#include <variant>
#include <type_traits>
#include "HelperFunctions.hpp"
#include "ComponentData.hpp"

template<typename T>
class AnimationPropertyKeyframe
{
private:
	float m_time;
	T m_value;
public:

private:
public:
	AnimationPropertyKeyframe(const T& value, const float& time) : 
		m_value(value), m_time(time) {}

	const float& GetTime() const { return m_time; }
	const T& GetValue() const { return m_value; }

	std::string ToString() const
	{
		return std::format("[Time: {} Value: {}]", 
			std::to_string(m_time), Utils::TryToString<T>(m_value));
	}
};

template<typename T>
struct AnimationProperty
{
	std::vector<AnimationPropertyKeyframe<T>> m_Keyframes;
	size_t m_KeyframeIndex;
	T& m_ComponentPropertyRef;
	bool& m_ComponentDataMutationFlagRef;

	AnimationProperty(T& compoenentPropertyRef, bool& componentDataMutationRef, const std::vector<AnimationPropertyKeyframe<T>> keyframes) :
		m_ComponentPropertyRef(compoenentPropertyRef), m_ComponentDataMutationFlagRef(componentDataMutationRef), m_Keyframes(keyframes), m_KeyframeIndex(0) {}

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
class AnimatorData : public ComponentData
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

void from_json(const Json& json, AnimationPropertyVariant& var);
void to_json(Json& json, const AnimationPropertyVariant& var);

