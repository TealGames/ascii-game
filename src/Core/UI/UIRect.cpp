#include "pch.hpp"
#include "Core/UI/UIRect.hpp"
#include "Utils/Debug.hpp"

namespace Engine::UI
{
	UIRect::UIRect() : UIRect(UI_RECT_BOTTOM_LEFT, UI_RECT_TOP_RIGHT) {}
	UIRect::UIRect(const NormalizedVec2& size) : UIRect(UI_RECT_BOTTOM_LEFT, size) {}
	UIRect::UIRect(const NormalizedVec2& bottomLeft, const NormalizedVec2& size, const NormalizedVec2& pivot) :
		m_bottomLeftPos(bottomLeft), m_topRightPos(bottomLeft + size), m_Pivot(pivot)
	{
		ENGINE_ASSERT(HasValidSize(), "Tried to create a relative GUI rect with invalid size. "
			"Bottom left:{} size:{}", m_bottomLeftPos.ToString(), size.ToString());
	}

	bool UIRect::HasValidSize() const
	{
		const NormalizedVec2 size = GetSize();
		return size.m_X != 0 && size.m_Y != 0;
	}

	void UIRect::SetSizeWithPivot(const NormalizedVec2& size, const NormalizedVec2& pivot)
	{
		if (size.m_X == 0 || size.m_Y == 0)
		{
			LogError(std::format("Tried to set size for relative "
				"gui:{} rect to:{} which is not allowed", ToString(), size.ToString()));
			return;
		}
		//Note: this is to ensure that if we have 0 size, we max it out so new size would not be in an irregular state
		if (!HasValidSize()) SetMaxSize();

		const UIRect originalRect = *this;
		const Vec2 sizeDelta = (size - GetSize()).AsVec2();

		//These determine magnitude AND sign of the BL and TR pos contribute to increasing/decreasing size based on pivot
		Vec2 bottomLeftContrib = (pivot - UI_RECT_BOTTOM_LEFT).AsVec2();
		Vec2 topRightContrib = (pivot - UI_RECT_TOP_RIGHT).AsVec2();

		Vec2 newBottomLeftPos = m_bottomLeftPos.AsVec2() + bottomLeftContrib * sizeDelta;
		Vec2 passedBounds = newBottomLeftPos - UI_RECT_BOTTOM_LEFT.AsVec2();
		//If we went too far left or too far down passed bottom left, we must apply the amount it went passed to the top right
		//NOTE: this can ONLY happen if the size INCREASES
		if (passedBounds.m_X < 0.0f || passedBounds.m_Y < 0.0f)
		{
			//since bottom left expansion means X DECREASES and Y DECREASES, for top right it is opposite
			topRightContrib -= passedBounds;
			newBottomLeftPos = UI_RECT_BOTTOM_LEFT.AsVec2();
		}

		Vec2 newtopRightPos = m_topRightPos.AsVec2() + topRightContrib * sizeDelta;
		passedBounds = newtopRightPos - UI_RECT_TOP_RIGHT.AsVec2();
		//Same as bottomLeft but for top right. NOTE: BOTH BL AND TR NEEDING ADJUSTING DURING ONE CALL SHOULD NEVER HAPPEN
		if (passedBounds.m_X > 0.0f || passedBounds.m_Y > 0.0f)
		{
			bottomLeftContrib -= passedBounds;
			newtopRightPos = UI_RECT_TOP_RIGHT.AsVec2();
		}

		m_bottomLeftPos = newBottomLeftPos;
		m_topRightPos = newtopRightPos;

		ENGINE_ASSERT(HasValidSize(), "Tried to set size for relative "
			"gui rect to:{} but resulted in invalid size:{}", size.ToString(), ToString());
	}
	void UIRect::SetSize(const NormalizedVec2& size) { SetSizeWithPivot(size, m_Pivot); }

	void UIRect::SetMaxSize()
	{
		m_bottomLeftPos = UI_RECT_BOTTOM_LEFT;
		m_topRightPos = UI_RECT_TOP_RIGHT;
	}
	void UIRect::SetBottomLeft(const NormalizedVec2& bottomLeft, const bool maintainSize)
	{
		const NormalizedVec2 posDelta = bottomLeft - m_bottomLeftPos;
		m_bottomLeftPos = bottomLeft;
		if (maintainSize) m_topRightPos += posDelta;

		ENGINE_ASSERT(HasValidSize(), "Tried to set top left pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString());
	}
	void UIRect::SetBottomRight(const NormalizedVec2& bottomRight, const bool maintainSize)
	{
		const NormalizedVec2 posDelta = bottomRight - GetBottomRightPos();
		m_bottomLeftPos.m_Y = bottomRight.m_Y;
		m_topRightPos.m_X = bottomRight.m_X;
		if (maintainSize)
		{
			m_bottomLeftPos.m_X += posDelta.m_X;
			m_topRightPos.m_Y += posDelta.m_Y;
		}

		ENGINE_ASSERT(HasValidSize(), "Tried to set bottom right pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString());
	}
	void UIRect::SetTopRight(const NormalizedVec2& topRight, const bool maintainSize)
	{
		const NormalizedVec2 posDelta = topRight - m_topRightPos;
		m_topRightPos = topRight;
		if (maintainSize) m_bottomLeftPos += posDelta;

		ENGINE_ASSERT(HasValidSize(), "Tried to set bottom right pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString());
	}
	void UIRect::SetTopLeft(const NormalizedVec2& topLeft, const bool maintainSize)
	{
		const NormalizedVec2 posDelta = topLeft - GetTopLeftPos();
		m_bottomLeftPos.m_X = topLeft.m_X;
		m_topRightPos.m_Y = topLeft.m_Y;
		if (maintainSize)
		{
			m_bottomLeftPos.m_Y += posDelta.m_Y;
			m_topRightPos.m_X += posDelta.m_X;
		}

		ENGINE_ASSERT(HasValidSize(), "Tried to set top left pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString());
	}
	void UIRect::SetBoundsBLTR(const NormalizedVec2& bottomLeft, const NormalizedVec2& topRight)
	{
		ENGINE_ASSERT(topRight > bottomLeft, "Attempted to SetBounds using BottomLeft:{} TopRight:{} "
			"but TopRight is not strictly greater than BottomLeft", bottomLeft.ToString(), topRight.ToString());

		m_bottomLeftPos = bottomLeft;
		m_topRightPos = topRight;

		ENGINE_ASSERT(HasValidSize(), "Tried to set bounds:[BottomLeft:{} TopRight:{}] for relative "
			"gui rect but resulted in invalid size:{}", bottomLeft.ToString(), topRight.ToString(), ToString());
	}
	void UIRect::SetBoundsTLBR(const NormalizedVec2& topLeft, const NormalizedVec2& bottomRight)
	{
		ENGINE_ASSERT(topLeft.m_Y > bottomRight.m_Y && bottomRight.m_X > topLeft.m_X,
			"Attempted to SetBounds using TopLeft:{} BottomRight:{} "
			"but one or more components are invalid", topLeft.ToString(), bottomRight.ToString());

		SetTopLeft(topLeft, false);
		SetBottomRight(bottomRight, false);

		ENGINE_ASSERT(HasValidSize(), "Tried to set bounds:[TopLeft:{} BottomRight:{}] for relative "
			"gui rect but resulted in invalid size:{}", topLeft.ToString(), bottomRight.ToString(), ToString());
	}

	bool UIRect::ContainsPos(const NormalizedVec2& pos) const
	{
		return m_bottomLeftPos <= pos && pos <= m_topRightPos;
	}

	NormalizedVec2 UIRect::GetSize() const { return m_topRightPos - m_bottomLeftPos; }

	const NormalizedVec2& UIRect::GetBottomLeftPos() const { return m_bottomLeftPos; }
	NormalizedVec2 UIRect::GetTopLeftPos() const { return NormalizedVec2(m_bottomLeftPos.m_X, m_topRightPos.m_Y); }
	const NormalizedVec2& UIRect::GetTopRightPos() const { return m_topRightPos; }
	NormalizedVec2 UIRect::GetBottomRightPos() const { return NormalizedVec2(m_topRightPos.m_X, m_bottomLeftPos.m_Y); }

	Mat3 UIRect::CalculateModelMatrix() const
	{
		return Math::CalculateUIModelMatrix(nullptr, m_bottomLeftPos.AsVec2(),
			GetSize().AsVec2(), m_Pivot.AsVec2());
	}

	std::string UIRect::ToString() const
	{
		return std::format("[UIRect BL:{} TR:{} size:{} Pivot:{}]",
			m_bottomLeftPos.ToString(), m_topRightPos.ToString(), GetSize().ToString(), m_Pivot.ToString());
	}

	UIRect CalculateWorldRect(const UIRect& parentWorldRect, const UIRect& localRect)
	{
		const NormalizedVec2 parentSize = parentWorldRect.GetSize();
		const NormalizedVec2 localSize = localRect.GetSize();

		const NormalizedVec2 worldBottomLeft = parentWorldRect.GetBottomLeftPos() + localRect.GetBottomLeftPos() * parentSize;
		const NormalizedVec2 worldSize = parentSize * localSize;
		//NOTE: the pivot should NOT change to world space and should remain in local space
		return UIRect(worldBottomLeft, worldSize, localRect.m_Pivot);
	}
}

