#pragma once
#include <vector>
#include "VisualData.hpp"
#include "ComponentData.hpp"

/// <summary>
/// Stores the whole entire visual frame. This is meant for the user interface for 
/// supplying the frames to the sprite aniamtor
/// </summary>
struct SpriteAnimationFrame
{
	float m_Time;
	VisualData m_VisualFrame;

	SpriteAnimationFrame();
	SpriteAnimationFrame(const float& time, const VisualData& frame);
};

/// <summary>
/// Stores a delta from the past frame. This is meant for internal storage of the 
/// frames and provides optimization
/// </summary>
struct SpriteAnimationDelta
{
	float m_Time;
	VisualDataPositions m_VisualDelta;

	SpriteAnimationDelta();
	SpriteAnimationDelta(const float& time, const VisualDataPositions& data);

	std::string ToString() const;
};

struct SpriteAnimatorData : public ComponentData
{
	std::vector<SpriteAnimationDelta> m_VisualDeltas;
	size_t m_VisualDeltaIndex;
	/// <summary>
	/// The duration in seconds of a single animation loop
	/// </summary>
	float m_SingleLoopLength;
	/// <summary>
	/// The current time of the animation relative to its beginning
	/// </summary>
	float m_NormalizedTime;
	float m_AnimationSpeed;
	bool m_Loop;

	SpriteAnimatorData();
	SpriteAnimatorData(const Json& json);
	SpriteAnimatorData(const std::vector<SpriteAnimationFrame>& frames,
		const float& animationSpeed, const float& loopTime, const bool& loop);

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

