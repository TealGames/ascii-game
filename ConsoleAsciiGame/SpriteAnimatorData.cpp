#include "pch.hpp"
#include "SpriteAnimatorData.hpp"
#include "JsonSerializers.hpp"
#include "HelperFunctions.hpp"
#include "SpriteAnimationAsset.hpp"

SpriteAnimatorData::SpriteAnimatorData() : ComponentData(), m_animations(), m_playingAnimation(nullptr) {}
SpriteAnimatorData::SpriteAnimatorData(const Json& json) : SpriteAnimatorData()
{
	Deserialize(json);
}

void SpriteAnimatorData::AddAnimation(SpriteAnimationAsset& animation)
{
	m_animations.emplace(animation.GetAnimation().m_Name, &animation);
}

bool SpriteAnimatorData::HasAnimation(const std::string& name) const
{
	return m_animations.find(name) != m_animations.end();
}

bool SpriteAnimatorData::IsPlayingAnimation() const
{
	return m_playingAnimation != nullptr;
}
const SpriteAnimation* SpriteAnimatorData::TryGetPlayingAnimation() const
{
	if (m_playingAnimation == nullptr) return nullptr;
	return &(m_playingAnimation->GetAnimation());
}
SpriteAnimation* SpriteAnimatorData::TryGetPlayingAnimationMutable()
{
	if (m_playingAnimation == nullptr) return nullptr;
	return &(m_playingAnimation->GetAnimationMutable());
}
bool SpriteAnimatorData::TryPlayAnimation(const std::string& name)
{
	TryStopCurrentAnimation();

	auto animIt = m_animations.find(name);
	if (animIt == m_animations.end()) return false;

	m_playingAnimation = animIt->second;
	return true;
}
bool SpriteAnimatorData::TryStopCurrentAnimation()
{
	if (!IsPlayingAnimation()) return false;

	m_playingAnimation = nullptr;
	return true;
}
bool SpriteAnimatorData::TryStopAnimation(const std::string& name)
{
	if (!IsPlayingAnimation() || m_playingAnimation->GetAnimation().m_Name!=name) return false;

	m_playingAnimation = nullptr;
	return true;
}

void SpriteAnimatorData::InitFields()
{
	//m_Fields = {ComponentField("Loop", &m_Loop), ComponentField("Speed", &m_AnimationSpeed)};
	m_Fields = {};
}

//std::vector<std::string> SpriteAnimatorData::GetDependencyFlags() const
//{
//	return {};
//}

void SpriteAnimatorData::Deserialize(const Json& json)
{
	SpriteAnimationAsset* animAsset = nullptr;
	for (const Json& animationJson : json.at("Animations").get<std::vector<Json>>())
	{
		animAsset= TryDeserializeTypeAsset<SpriteAnimationAsset>(animationJson);
		m_animations.emplace(animAsset->GetAnimation().m_Name, animAsset);
	}
}
Json SpriteAnimatorData::Serialize()
{
	std::vector<Json> serializedAssets = {};
	for (const auto& animation : m_animations)
	{
		if (animation.second == nullptr) continue;
		serializedAssets.push_back(TrySerializeAsset(*animation.second));
	}
	return { "Animations", serializedAssets };
}

std::string SpriteAnimatorData::ToString() const
{
	std::vector<std::string> animtionsStr = {};
	for (const auto& anim : m_animations)
	{
		animtionsStr.push_back(anim.second->GetAnimation().ToString());
	}
	return std::format("[SpriteAniamtor Anims:{}]", 
		Utils::ToStringIterable<std::vector<std::string>, std::string>(animtionsStr));
}
