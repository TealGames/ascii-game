#pragma once
#include <vector>
#include "VisualData.hpp"
#include "ComponentData.hpp"
#include <unordered_map>

class SpriteAnimationAsset;
class SpriteAnimation;

class SpriteAnimatorData : public ComponentData
{
private:
	//TODO: replace with sparse set for optimization
	std::unordered_map<std::string, SpriteAnimationAsset*> m_animations;
	SpriteAnimationAsset* m_playingAnimation;

public:

private:
public:
	SpriteAnimatorData();
	SpriteAnimatorData(const Json& json);

	void AddAnimation(SpriteAnimationAsset& animationAsset);

	template<typename ...Args>
	void EmplaceAnimation(Args&&... args)
	{
		auto tuple = std::make_tuple(args...);
		m_animations.emplace(std::get<0>(tuple), std::forward(args...));
	}

	bool HasAnimation(const std::string& name) const;

	bool IsPlayingAnimation() const;
	const SpriteAnimation* TryGetPlayingAnimation() const;
	SpriteAnimation* TryGetPlayingAnimationMutable();
	/// <summary>
	/// Will attempt to play an animation by this name (if it exists).
	/// Note: since only one animation can play at a time any other playing are stopped
	/// Note: returns false if no animation begun to play and returns true if began playing
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	bool TryPlayAnimation(const std::string& name);
	/// <summary>
	/// Tries to stop the playing animation (if there is any playing) by this name
	/// Return false if none stopped and true if one was stopped
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	bool TryStopAnimation(const std::string& name);
	bool TryStopCurrentAnimation();

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

