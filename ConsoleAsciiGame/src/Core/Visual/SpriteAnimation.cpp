#include "pch.hpp"
#include "Core/Visual/SpriteAnimation.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/HelperFunctions.hpp"

SpriteAnimationFrame::SpriteAnimationFrame() : SpriteAnimationFrame(0, {}) {}
SpriteAnimationFrame::SpriteAnimationFrame(const float& time, const VisualData& frame) :
	m_Time(time), m_VisualFrame(frame) {}

std::string SpriteAnimationFrame::ToString() const
{
	return std::format("[SpriteAnimationFrame Time:{} Visual:{}]", std::to_string(m_Time), m_VisualFrame.ToString());
}

SpriteAnimationDelta::SpriteAnimationDelta() : SpriteAnimationDelta(0, {}, {}) {}
SpriteAnimationDelta::SpriteAnimationDelta(const float& time, const VisualDataPositions& data, const Vec2Int& visualSize) :
	m_Time(time), m_VisualDelta(data), m_VisualSize(visualSize) {}

std::string SpriteAnimationDelta::ToString() const
{
	return std::format("[Time:{}, VisualDelta:{}]", std::to_string(m_Time),
		Utils::ToStringIterable<VisualDataPositions, TextCharArrayPosition>(m_VisualDelta));
}

SpriteAnimation::SpriteAnimation() : SpriteAnimation(std::to_string(Utils::GenerateRandomInt(0, 100)), {}, 1, 1, true) {}
SpriteAnimation::SpriteAnimation(const std::string& name, const std::vector<SpriteAnimationFrame>& frames,
	const float& animationSpeed, const float& loopTime, const bool& loop) :
	m_Name(name), m_Frames(frames), m_FrameIndex(0), m_SingleLoopLength(loopTime),
	m_NormalizedTime(0), m_AnimationSpeed(animationSpeed), m_Loop(loop)
{

}

const VisualData& SpriteAnimation::GetVisualDataForFrame(const size_t index) const
{
	if (!Assert(0 <= index && index < m_Frames.size(), std::format("Tried to get visual data for frame "
		"at invalid index:{} but index is valid in range:[0,{})", std::to_string(index), std::to_string(m_Frames.size()))))
		throw std::invalid_argument("Invalid frame index");

	return m_Frames.at(index).m_VisualFrame;
}
const VisualData* SpriteAnimation::TryGetCurrentVisualData() const
{
	if (m_FrameIndex == -1 || m_FrameIndex >= m_Frames.size()) 
		return nullptr;

	return &(GetVisualDataForFrame(m_FrameIndex));
}

//void SpriteAnimation::SetVisualsFromFrames(const std::vector<SpriteAnimationFrame>& frames)
//{
//	SpriteAnimationDelta visualDelta;
//
//	VisualDataPositions currentVisual = {};
//	const VisualData* currentData = nullptr;
//	const VisualData* prevData = nullptr;
//
//	for (int i = 0; i < frames.size(); i++)
//	{
//		currentData = &(frames[i].m_VisualFrame);
//		if (currentData == nullptr) continue;
//
//		m_Frames.push_back(*currentData);
//		visualDelta = { frames[i].m_Time, {}, currentData->GetBufferSize()};
//		currentVisual = {};
//
//		if (i > 0) prevData = &(frames[i - 1].m_VisualFrame);
//
//		for (int r = 0; r < currentData->m_Text.GetHeight(); r++)
//		{
//			for (int c = 0; c < currentData->m_Text.GetWidth(); c++)
//			{
//				if (prevData != nullptr)
//				{
//					if (r >= prevData->m_Text.GetHeight() || c >= prevData->m_Text.GetWidth()) continue;
//					if (currentData->m_Text.GetAtUnsafe({ r, c }) == prevData->m_Text.GetAtUnsafe({ r, c })) continue;
//				}
//
//				currentVisual.emplace_back(Array2DPosition{ r,c }, currentData->m_Text.GetAtUnsafe({ r, c }));
//			}
//		}
//		visualDelta.m_VisualDelta = currentVisual;
//		m_FrameDeltas.push_back(visualDelta);
//	}
//}

std::string SpriteAnimation::ToString() const
{
	return std::format("[SpriteAnimation Loop:{} Speed:{} Length:{} Frames:{}]",
		std::to_string(m_Loop), std::to_string(m_AnimationSpeed), std::to_string(m_SingleLoopLength),
		Utils::ToStringIterable<std::vector<SpriteAnimationFrame>, SpriteAnimationFrame>(m_Frames));
}