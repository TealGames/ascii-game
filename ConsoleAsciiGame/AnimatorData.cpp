#include "pch.hpp"
#include "AnimatorData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "JsonSerializers.hpp"

AnimatorData::AnimatorData() : AnimatorData({}, 0, 1, false) {}

AnimatorData::AnimatorData(const Json& json) : AnimatorData()
{
	Deserialize(json);
}

AnimatorData::AnimatorData(const std::vector<AnimationPropertyVariant>& properties, const float& animationTime, const float& speed, const bool& loop) :
	ComponentData(),
	m_Properties(properties), m_AnimationSpeed(speed), m_NormalizedTime(0), m_KeyframeIndex(0), m_AnimationLength(animationTime), m_Loop(loop)
{
	if (!Assert(this, !Utils::ApproximateEqualsF(m_AnimationSpeed, 0),
		std::format("Tried to set animator data:{} with animation speed of zero!", 
		Utils::ToStringIterable<std::vector<AnimationPropertyVariant>, AnimationPropertyVariant>(m_Properties)))) 
		return;

	//if (!Assert(this, !m_Properties.empty(),
	//	std::format("Tried to set animator data:{} but there are no key frames", 
	//	Utils::ToStringIterable<std::vector<AnimationPropertyVariant>, AnimationPropertyVariant>(m_Properties)))) 
	//	return;

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

void AnimatorData::SetAnimationSpeed(const float& speed)
{
	m_AnimationSpeed = std::abs(speed);
}

const float& AnimatorData::GetTimeLength() const
{
	return m_AnimationLength;
}

void AnimatorData::InitFields()
{
	m_Fields = { ComponentField("Loop", &m_Loop), ComponentField("Speed", 
		(std::function<void(float)>)[this](float speed)->void {SetAnimationSpeed(speed); }, &m_AnimationSpeed) };
}
std::vector<std::string> AnimatorData::GetDependencyFlags() const
{
	return {};
}

void AnimatorData::Deserialize(const Json& json)
{
	m_Loop = json.at("Loop").get<bool>();
	SetAnimationSpeed(json.at("Speed").get<float>());
	m_AnimationLength = json.at("Time").get<float>();
	m_Properties = json.at("Properties").get<std::vector<AnimationPropertyVariant>>();
}
Json AnimatorData::Serialize()
{
	return { {"Loop", m_Loop}, {"Speed", m_AnimationSpeed}, {"Time", m_AnimationLength}, {"Properties", m_Properties}};
}

std::string AnimatorData::ToString() const
{
	std::vector<std::string> propertyStrs = {};
	for (auto& property : m_Properties)
	{
		std::visit([&property, &propertyStrs](auto&& value) -> void
			{
				using PropertyConvertedType = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
				using ExtractedType = AnimationPropertyType<PropertyConvertedType>::Type;

				propertyStrs.push_back(std::get<PropertyConvertedType>(property).ToString());

			}, property);
		
	}
	return std::format("[Animator Loop:{} Speed:{} Properties:{}]", 
		std::to_string(m_Loop), std::to_string(m_AnimationSpeed), 
		Utils::ToStringIterable<std::vector<std::string>, std::string>(propertyStrs));
}