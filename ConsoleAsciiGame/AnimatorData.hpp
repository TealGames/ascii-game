#pragma once
#include <format>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <variant>
#include <type_traits>
#include "ComponentType.hpp"
#include "HelperFunctions.hpp"
#include "ComponentData.hpp"

constexpr ComponentType SUPPORTED_ANIMATOR_COMPONENTS = ComponentType::LightSource;

using AnimationPropertyValue = std::variant<int, float, std::uint8_t>;
struct AnimationProperty
{
	ComponentType m_ComponentType;
	std::string m_PropertyName;
	AnimationPropertyValue m_Value;

	AnimationProperty(const ComponentType& type, 
		const std::string& name, const AnimationPropertyValue& value);

	std::string ToString() const;

	bool operator==(const AnimationProperty& other) const = default;
	bool operator<(const AnimationProperty& other) const;
};

struct AnimationKeyframe
{
	float m_Time;
	std::unordered_map<std::string, AnimationProperty> m_Properties;

	AnimationKeyframe();
	AnimationKeyframe(const std::vector<AnimationProperty>& properties, const float& time);

	std::string ToString() const;
};

struct AnimatorData : public ComponentData
{
	std::vector<AnimationKeyframe> m_Keyframes;
	float m_AnimationSpeed;
	float m_NormalizedTime;
	size_t m_KeyframeIndex;
	bool m_Loop;

	float m_EndTime;

	AnimatorData();
	AnimatorData(const std::vector<AnimationKeyframe>& frames, 
		const float& speed, const bool& loop);
};

