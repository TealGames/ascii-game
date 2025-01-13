#include "pch.hpp"
#include "AnimatorData.hpp"
#include "HelperFunctions.hpp"

AnimationProperty::AnimationProperty(const ComponentType& type, const std::string& name, const AnimationPropertyValue& value) :
	m_ComponentType(type), m_PropertyName(name), m_Value(value)
{
	Utils::Assert(this, (m_ComponentType & SUPPORTED_ANIMATOR_COMPONENTS) != ComponentType::None,
		std::format("Tried to create an animation property with an component type that is not supported: {}", ::ToString(type)));
}
bool AnimationProperty::operator<(const AnimationProperty& other) const
{
	return m_PropertyName < other.m_PropertyName;
}
std::string AnimationProperty::ToString() const
{
	return std::format("[Component: {} Property: {}]", ::ToString(m_ComponentType), m_PropertyName);
}

AnimationKeyframe::AnimationKeyframe() : m_Properties{}, m_Time(0) {}
AnimationKeyframe::AnimationKeyframe(const std::vector<AnimationProperty>& properties, const float& time) :
	m_Properties{}, m_Time(time)
{
	for (const auto& property : properties)
	{
		m_Properties.emplace(property.m_PropertyName, property);
	}
}

std::string AnimationKeyframe::ToString() const
{
	return std::format("[{}sec]", std::to_string(m_Time));
}

AnimatorData::AnimatorData() : 
	ComponentData(), m_Keyframes{}, m_AnimationSpeed(), m_NormalizedTime(0), m_KeyframeIndex(0), m_EndTime(0), m_Loop(false) {}

AnimatorData::AnimatorData(const std::vector<AnimationKeyframe>& frames, const float& speed, const bool& loop) :
	ComponentData(), m_Keyframes(frames), m_AnimationSpeed(speed), m_NormalizedTime(0), m_KeyframeIndex(0), m_EndTime(0), m_Loop(loop)
{
	if (!Utils::Assert(this, !Utils::ApproximateEqualsF(m_AnimationSpeed, 0),
		std::format("Tried to set animator data:{} with animation speed of zero!", 
		Utils::ToStringIterable<std::vector<AnimationKeyframe>, AnimationKeyframe>(m_Keyframes)))) return;

	if (!Utils::Assert(this, !m_Keyframes.empty(),
		std::format("Tried to set animator data:{} but there are no key frames", 
		Utils::ToStringIterable<std::vector<AnimationKeyframe>, AnimationKeyframe>(m_Keyframes)))) return;

	if (!Utils::Assert(this, Utils::ApproximateEqualsF(m_Keyframes.front().m_Time, 0),
		std::format("Tried to set animator data:{} but the first keyframe is not set at time 0 which is required!",
			Utils::ToStringIterable<std::vector<AnimationKeyframe>, AnimationKeyframe>(m_Keyframes)))) return;

	m_EndTime = m_Keyframes.back().m_Time;
}
