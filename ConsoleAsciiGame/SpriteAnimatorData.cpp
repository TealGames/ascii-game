#include "pch.hpp"
#include "SpriteAnimatorData.hpp"

SpriteAnimationFrame::SpriteAnimationFrame() : SpriteAnimationFrame(0, {}) {}
SpriteAnimationFrame::SpriteAnimationFrame(const float& time, const VisualData& frame) :
	m_Time(time), m_VisualFrame(frame) {}

SpriteAnimationDelta::SpriteAnimationDelta() : SpriteAnimationDelta(0, {}) {}
SpriteAnimationDelta::SpriteAnimationDelta(const float& time, const VisualDataPositions& data) :
	m_Time(time), m_VisualDelta(data) {}

SpriteAnimatorData::SpriteAnimatorData() : SpriteAnimatorData({}, 1, 1, false) {}
SpriteAnimatorData::SpriteAnimatorData(const Json& json) : SpriteAnimatorData()
{
	Deserialize(json);
}

SpriteAnimatorData::SpriteAnimatorData(const std::vector<SpriteAnimationFrame>& frames, const float& animationSpeed, const float& loopTime, const bool& loop) :
	ComponentData(HighestDependecyLevel::None),
	m_VisualDeltas{}, m_VisualDeltaIndex(0), m_SingleLoopLength(loopTime), m_NormalizedTime(0), m_AnimationSpeed(animationSpeed), m_Loop(loop)
{
	SpriteAnimationDelta visualDelta;

	VisualDataPositions currentVisual = {};
	const VisualData* currentData = nullptr;
	const VisualData* prevData = nullptr;

	for (int i=0; i<frames.size(); i++)
	{
		currentData = &(frames[i].m_VisualFrame);
		if (currentData == nullptr) continue;

		visualDelta = { frames[i].m_Time, {} };
		currentVisual = {};

		if (i>0) prevData = &(frames[i - 1].m_VisualFrame);

		for (int r = 0; r < currentData->m_Text.GetHeight(); r++)
		{
			for (int c = 0; c < currentData->m_Text.GetWidth(); c++)
			{
				if (prevData != nullptr)
				{
					if (r >= prevData->m_Text.GetHeight() || c >= prevData->m_Text.GetWidth()) continue;
					if (currentData->m_Text.GetAtUnsafe({r, c}) == prevData->m_Text.GetAtUnsafe({r, c})) continue;
				}
				
				currentVisual.m_Data.emplace_back(Array2DPosition{r,c}, currentData->m_Text.GetAtUnsafe({r, c}));
			}
		}
		visualDelta.m_VisualDelta = currentVisual;
		m_VisualDeltas.push_back(visualDelta);
	}
}

void SpriteAnimatorData::InitFields()
{
	m_Fields = {ComponentField("Loop", &m_Loop), ComponentField("Speed", &m_AnimationSpeed)};
}

void SpriteAnimatorData::Deserialize(const Json& json)
{
	m_Loop = json.at("Loop").get<bool>();
	m_AnimationSpeed = json.at("Speed").get<float>();
}
Json SpriteAnimatorData::Serialize()
{
	return { {"Loop", m_Loop}, {"Speed", m_AnimationSpeed} };
}

std::string SpriteAnimatorData::ToString() const
{
	return std::format("[SpriteAnimator Loop:{} Speed:{}]",
		std::to_string(m_Loop), std::to_string(m_AnimationSpeed));
}
