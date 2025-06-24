#pragma once
#include "VisualData.hpp"

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

	std::string ToString() const;
};

/// <summary>
/// Stores a delta from the past frame. This is meant for internal storage of the 
/// frames and provides optimization
/// </summary>
struct SpriteAnimationDelta
{
	float m_Time;
	VisualDataPositions m_VisualDelta;
	Vec2Int m_VisualSize;

	SpriteAnimationDelta();
	SpriteAnimationDelta(const float& time, const VisualDataPositions& data, const Vec2Int& visualSize);

	std::string ToString() const;
};

class SpriteAnimation
{
private:
public:
	std::string m_Name;
	std::vector<SpriteAnimationFrame> m_Frames;
	size_t m_FrameIndex;
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

private:
public:
	SpriteAnimation();
	SpriteAnimation(const std::string& name, const std::vector<SpriteAnimationFrame>& frames,
		const float& animationSpeed, const float& loopTime, const bool& loop);

	//void SetVisualsFromFrames(const std::vector<SpriteAnimationFrame>& frames);

	const VisualData& GetVisualDataForFrame(const size_t index) const;
	const VisualData* TryGetCurrentVisualData() const;

	std::string ToString() const;
};

