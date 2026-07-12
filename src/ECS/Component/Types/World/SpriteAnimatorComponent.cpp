#include "pch.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorComponent.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Asset/SpriteAnimationAsset.hpp"

namespace Engine::Animation
{
	SpriteAnimatorComponent::SpriteAnimatorComponent() : Component(), m_animations(), m_playingAnimation(nullptr) {}

	void SpriteAnimatorComponent::AddAnimation(SpriteAnimationAsset& animation)
	{
		m_animations.emplace(animation.GetAnimation().m_Name, &animation);
	}

	bool SpriteAnimatorComponent::HasAnimation(const std::string& name) const
	{
		return m_animations.find(name) != m_animations.end();
	}

	bool SpriteAnimatorComponent::IsPlayingAnimation() const
	{
		return m_playingAnimation != nullptr;
	}
	const SpriteAnimation* SpriteAnimatorComponent::TryGetPlayingAnimation() const
	{
		if (m_playingAnimation == nullptr) return nullptr;
		return &(m_playingAnimation->GetAnimation());
	}
	SpriteAnimation* SpriteAnimatorComponent::TryGetPlayingAnimationMutable()
	{
		if (m_playingAnimation == nullptr) return nullptr;
		return &(m_playingAnimation->GetAnimationMutable());
	}
	bool SpriteAnimatorComponent::TryPlayAnimation(const std::string& name)
	{
		TryStopCurrentAnimation();

		auto animIt = m_animations.find(name);
		if (animIt == m_animations.end()) return false;

		m_playingAnimation = animIt->second;
		return true;
	}
	bool SpriteAnimatorComponent::TryStopCurrentAnimation()
	{
		if (!IsPlayingAnimation()) return false;

		m_playingAnimation = nullptr;
		return true;
	}
	bool SpriteAnimatorComponent::TryStopAnimation(const std::string& name)
	{
		if (!IsPlayingAnimation() || m_playingAnimation->GetAnimation().m_Name != name) return false;

		m_playingAnimation = nullptr;
		return true;
	}

	void SpriteAnimatorComponent::InitFields()
	{
		//m_Fields = {ComponentField("Loop", &m_Loop), ComponentField("Speed", &m_AnimationSpeed)};
		m_Fields = {};
	}
	void SpriteAnimatorComponent::Serialize(Serialization::Serializer& serializer) const
	{
		std::vector<Serialization::SerializedAsset> serializedAssets = {};
		for (const auto& animationAssetPair : m_animations) 
			serializedAssets.push_back(Serialization::TrySerializeAsset(animationAssetPair.second));

		serializer.AddProperty("Animations", serializedAssets);
	}
	void SpriteAnimatorComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		std::vector<Serialization::SerializedAsset> serializedAssets = {};
		deserializer.GetProperty("Animations", &serializedAssets);
		for (const auto& serializedAsset : serializedAssets)
		{
			SpriteAnimationAsset* maybeAsset = Serialization::TryDeserializeTypeAsset<SpriteAnimationAsset>(serializedAsset);
			if (maybeAsset == nullptr)
			{
				LogError(std::format("Attempted to deserialize sprite animation component but serialized asset: {} "
					"could not be deserialized into valid object", serializedAsset.ToString()));
				continue;
			}
			AddAnimation(*maybeAsset);
		}
	}

	std::string SpriteAnimatorComponent::ToString() const
	{
		std::vector<std::string> animtionsStr = {};
		for (const auto& anim : m_animations)
		{
			animtionsStr.push_back(anim.second->GetAnimation().ToString());
		}
		return std::format("[SpriteAniamtor Anims:{}]",
			::Utils::ToStringIterable(animtionsStr));
	}

}
