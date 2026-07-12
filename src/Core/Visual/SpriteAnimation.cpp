#include "pch.hpp"
#include "Core/Visual/SpriteAnimation.hpp"
#include "Utils/Debug.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Engine::Animation
{
	SpriteAnimationFrame::SpriteAnimationFrame() : SpriteAnimationFrame(0, {}) {}
	SpriteAnimationFrame::SpriteAnimationFrame(const float& time, const Rendering::VisualData& frame) :
		m_Time(time), m_VisualFrame(frame) {}

	std::string SpriteAnimationFrame::ToString() const
	{
		return std::format("[SpriteAnimationFrame Time:{} Visual:{}]", std::to_string(m_Time), m_VisualFrame.ToString());
	}

	SpriteAnimationDelta::SpriteAnimationDelta() : SpriteAnimationDelta(0, {}, {}) {}
	SpriteAnimationDelta::SpriteAnimationDelta(const float& time, const Rendering::VisualDataPositions& data, const Vec2Int& visualSize) :
		m_Time(time), m_VisualDelta(data), m_VisualSize(visualSize) {}

	std::string SpriteAnimationDelta::ToString() const
	{
		return std::format("[Time:{}, VisualDelta:{}]", std::to_string(m_Time),
			Utils::ToStringIterable(m_VisualDelta));
	}

	SpriteAnimation::SpriteAnimation() : SpriteAnimation(std::to_string(::Utils::GenerateRandomInt(0, 100)), {}, 1, 1, true) {}
	SpriteAnimation::SpriteAnimation(const std::string& name, const std::vector<SpriteAnimationFrame>& frames,
		const float& animationSpeed, const float& loopTime, const bool& loop) :
		m_Name(name), m_Frames(frames), m_FrameIndex(0), m_SingleLoopLength(loopTime),
		m_NormalizedTime(0), m_AnimationSpeed(animationSpeed), m_Loop(loop)
	{

	}

	const Rendering::VisualData& SpriteAnimation::GetVisualDataForFrame(const size_t index) const
	{
		ENGINE_ASSERT(0 <= index && index < m_Frames.size(), "Tried to get visual data for frame "
			"at invalid index:{} but index is valid in range:[0,{})", std::to_string(index), std::to_string(m_Frames.size()));

		return m_Frames.at(index).m_VisualFrame;
	}
	const Rendering::VisualData* SpriteAnimation::TryGetCurrentVisualData() const
	{
		if (m_FrameIndex == -1 || m_FrameIndex >= m_Frames.size())
			return nullptr;

		return &(GetVisualDataForFrame(m_FrameIndex));
	}

	std::string SpriteAnimation::ToString() const
	{
		return std::format("[SpriteAnimation Loop:{} Speed:{} Length:{} Frames:{}]",
			std::to_string(m_Loop), std::to_string(m_AnimationSpeed), std::to_string(m_SingleLoopLength),
			Utils::ToStringIterable(m_Frames));
	}
}
