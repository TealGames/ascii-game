#include "pch.hpp"
#include "AnimatorData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

AnimatorData::AnimatorData() : 
	ComponentData(), m_Properties{}, m_AnimationSpeed(), m_NormalizedTime(0), m_KeyframeIndex(0), m_EndTime(0), m_Loop(false) {}

AnimatorData::AnimatorData(const std::vector<AnimationPropertyVariant>& properties, const float& animationTime, const float& speed, const bool& loop) :
	ComponentData(), m_Properties(properties), m_AnimationSpeed(speed), m_NormalizedTime(0), m_KeyframeIndex(0), m_EndTime(animationTime), m_Loop(loop)
{
	if (!Assert(this, !Utils::ApproximateEqualsF(m_AnimationSpeed, 0),
		std::format("Tried to set animator data:{} with animation speed of zero!", 
		Utils::ToStringIterable<std::vector<AnimationPropertyVariant>, AnimationPropertyVariant>(m_Properties)))) return;

	if (!Assert(this, !m_Properties.empty(),
		std::format("Tried to set animator data:{} but there are no key frames", 
		Utils::ToStringIterable<std::vector<AnimationPropertyVariant>, AnimationPropertyVariant>(m_Properties)))) return;

	//TODO: we need to check for some other conditions such as 
}

const bool& AnimatorData::GetDoLoop() const
{
	return m_Loop;
}

const std::vector<AnimationPropertyVariant>& AnimatorData::GetProperties() const
{
	return m_Properties;
}

const float& AnimatorData::GetAnimationSpeed() const
{
	return m_AnimationSpeed;
}

const float& AnimatorData::GetEndTime() const
{
	return m_EndTime;
}
