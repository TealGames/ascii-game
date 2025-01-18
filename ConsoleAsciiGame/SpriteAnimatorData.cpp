#include "pch.hpp"
#include "SpriteAnimatorData.hpp"

SpriteAnimationFrame::SpriteAnimationFrame() : SpriteAnimationFrame(0, {}) {}
SpriteAnimationFrame::SpriteAnimationFrame(const float& time, const VisualData& frame) :
	m_Time(time), m_VisualFrame(frame) {}

SpriteAnimationDelta::SpriteAnimationDelta() : SpriteAnimationDelta(0, {}) {}
SpriteAnimationDelta::SpriteAnimationDelta(const float& time, const VisualDataPositions& data) :
	m_Time(time), m_VisualDelta(data) {}

SpriteAnimatorData::SpriteAnimatorData(const std::vector<SpriteAnimationFrame>& frames, const float& animationSpeed, const float& endTime, const bool& loop) :
	m_VisualDeltas{}, m_VisualDeltaIndex(0), m_EndTime(endTime), m_NormalizedTime(0), m_AnimationSpeed(animationSpeed), m_Loop(loop)
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

		for (int r = 0; r < currentData->GetHeight(); r++)
		{
			for (int c = 0; c < currentData->GetWidth(); c++)
			{
				if (prevData != nullptr)
				{
					if (r >= prevData->GetHeight() || c >= prevData->GetWidth()) continue;
					if (currentData->GetAtUnsafe({r, c}) == prevData->GetAtUnsafe({r, c})) continue;
				}
				
				currentVisual.m_Data.emplace_back(Array2DPosition{r,c}, currentData->GetAtUnsafe({r, c}));
			}
		}
		visualDelta.m_VisualDelta = currentVisual;
		m_VisualDeltas.push_back(visualDelta);
	}
}
