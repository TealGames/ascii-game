#include "pch.hpp"
#include "RelativeUIRect.hpp"
#include "Debug.hpp"

RelativeUIRect::RelativeUIRect() : RelativeUIRect(NormalizedPosition::TOP_LEFT, NormalizedPosition(1, 1)) {}
RelativeUIRect::RelativeUIRect(const NormalizedPosition& size) : RelativeUIRect(NormalizedPosition::TOP_LEFT, size) {}
RelativeUIRect::RelativeUIRect(const NormalizedPosition& topLeft, const NormalizedPosition& size) :
	m_topLeftPos(topLeft), m_bottomRightPos(GetBottomRight(topLeft, size)) 
{
	if (HasInvalidSize())
	{
		Assert(false, std::format("Tried to create a relative GUI rect with invalid size. "
			"Top left:{} size:{}", topLeft.ToString(), size.ToString()));
	}
}

ScreenPosition GetSizeFromRelativePos(const NormalizedPosition& relativePos, const ScreenPosition& parentSize)
{
	return GetSizeFromFactor(relativePos.GetPos(), parentSize);
}
ScreenPosition GetSizeFromFactor(const Vec2& factor, const ScreenPosition& parentSize)
{
	return ScreenPosition(parentSize.m_X * factor.m_X, parentSize.m_Y * factor.m_Y);
}

bool RelativeUIRect::HasInvalidSize() const
{
	if (m_bottomRightPos.GetX() < m_topLeftPos.GetX() || m_bottomRightPos.GetY() > m_topLeftPos.GetY())
		return true;

	const NormalizedPosition size = GetSize();
	return size.GetX() == 0 || size.GetY() == 0;
}

void RelativeUIRect::SetSize(const NormalizedPosition& size)
{
	if (size.GetX()==0 || size.GetY()==0)
	{
		Assert(this, false, std::format("Tried to set size for relative "
			"gui:{} rect to:{} which is not allowed", ToString(), size.ToString()));
		return;
	}
	//Note: this is to ensure that if we have 0 size, we max it out so new size would not be in an irregular state
	if (HasInvalidSize()) SetMaxSize();

	//TODO: remove this line:
	const RelativeUIRect originalRect = *this;

	m_bottomRightPos= GetBottomRight(NormalizedPosition::TOP_LEFT, size);
	//Note: we want to do this order to ensure we get -y for down and +x for rightward dir
	const Vec2 topLeftAnchorCornerDelta = m_topLeftPos.GetPos() - NormalizedPosition::TOP_LEFT;
	const Vec2 netAvailableSpace = (Vec2(NormalizedPosition::MAX, NormalizedPosition::MAX) - size.GetPos()) - Abs(topLeftAnchorCornerDelta);

	//LogError(std::format("Net available space:{}", netAvailableSpace.ToString()));
	if (netAvailableSpace.m_X > 0 && netAvailableSpace.m_Y > 0)
	{
		const NormalizedPosition old = m_bottomRightPos;
		//LogError(std::format("BT before:{} detal:{}", m_bottomRightPos.ToString(), topLeftAnchorCornerDelta.ToString()));
		//Note: we have to do vector addition since delta can be negative which can be erased if done with normpos math
		m_bottomRightPos = m_bottomRightPos.GetPos() + topLeftAnchorCornerDelta;
		//LogError(std::format("BT after:{}", m_bottomRightPos.ToString()));

		NormalizedPosition fart = NormalizedPosition(0.5, 0.5);
		const Vec2 vec(0.23, 0.45);
		fart = fart + vec;
	/*	LogError(std::format("Converting br;{} -> {} delta:{} add:{} yas:{}", old.ToString(), m_bottomRightPos.ToString(), 
			topLeftAnchorCornerDelta.ToString(), (old+topLeftAnchorCornerDelta).ToString(), (fart).ToString()));*/
	}
	else m_topLeftPos = NormalizedPosition::TOP_LEFT;

	//const float deltaSizeX = m_topLeftPos.GetX() + size.GetX() - m_bottomRightPos.GetX();
	//const float deltaSizeY = m_topLeftPos.GetY() - size.GetY() - m_bottomRightPos.GetY();
	//const Vec2 newBottomRight = m_bottomRightPos.GetPos() + Vec2(deltaSizeX, deltaSizeY);

	////TODO: still does not consider case where rect might be in the middle and there is not enough space for top left and bottom right both
	////so we should then move in both topleft and bottom right equally
	//if (newBottomRight.m_X > 1 || newBottomRight.m_X < 0 || newBottomRight.m_Y > 1 || newBottomRight.m_Y < 0)
	//{
	//	m_topLeftPos.SetPos(m_topLeftPos.GetPos() + Vec2(-deltaSizeX,-deltaSizeY));
	//}
	//else if (keepTopLeft) m_bottomRightPos = GetBottomRight(m_topLeftPos, size);
	//else m_topLeftPos = GetTopLeft(m_bottomRightPos, size);

	if (HasInvalidSize())
	{
		Assert(this, false, std::format("Tried to set size for relative "
			"gui rect to:{} but resulted in invalid size:{} when converting rect:{} br:{} net:{}", 
			size.ToString(), ToString(), originalRect.ToString(), GetBottomRight(NormalizedPosition::TOP_LEFT, size).ToString(), netAvailableSpace.ToString()));
	}
}

void RelativeUIRect::SetMaxSize()
{
	m_bottomRightPos = NormalizedPosition::BOTTOM_RIGHT;
	m_topLeftPos = NormalizedPosition::TOP_LEFT;
}
void RelativeUIRect::SetTopLeft(const NormalizedPosition& topLeft)
{
	const Vec2 posDelta = topLeft.GetPos() - m_topLeftPos.GetPos();
	m_topLeftPos = topLeft;
	m_bottomRightPos = m_bottomRightPos.GetPos() + posDelta;

	if (HasInvalidSize())
	{
		Assert(this, false, std::format("Tried to set top left pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString()));
	}
}
void RelativeUIRect::SetBottomRight(const NormalizedPosition& bottomRight)
{
	const Vec2 posDelta = bottomRight.GetPos() - m_bottomRightPos.GetPos();
	m_bottomRightPos = bottomRight;
	m_topLeftPos= m_topLeftPos.GetPos() + posDelta;
	
	if (HasInvalidSize())
	{
		Assert(this, false, std::format("Tried to set bottom right pos for relative "
			"gui rect but it resulted in invalid size:{}", ToString()));
	}
}
void RelativeUIRect::SetBounds(const NormalizedPosition& topLeft, const NormalizedPosition& bottomRight)
{
	m_topLeftPos = topLeft;
	m_bottomRightPos = bottomRight;

	if (HasInvalidSize())
	{
		Assert(this, false, std::format("Tried to set bounds:[TopLeft:{} BottomRight:{}] for relative "
			"gui rect but resulted in invalid size:{}", topLeft.ToString(), bottomRight.ToString(), ToString()));
	}
}

NormalizedPosition RelativeUIRect::GetSize() const
{
	return GetSizeFromCorners(m_topLeftPos, m_bottomRightPos);
}
ScreenPosition RelativeUIRect::GetSize(const ScreenPosition parentSize) const
{
	return GetSizeFromRelativePos(GetSize(), parentSize);
}

const NormalizedPosition& RelativeUIRect::GetTopLeftPos() const
{
	return m_topLeftPos;
}
ScreenPosition RelativeUIRect::GetTopLeftPos(const ScreenPosition parentSize) const
{
	return GetSizeFromRelativePos(m_topLeftPos, parentSize);
}

const NormalizedPosition& RelativeUIRect::GetBottomRighttPos() const
{
	return m_bottomRightPos;
}
ScreenPosition RelativeUIRect::GetBottomRighttPos(const ScreenPosition parentSize) const
{
	return GetSizeFromRelativePos(m_bottomRightPos, parentSize);
}

std::string RelativeUIRect::ToString() const
{
	return std::format("[RelativeUIRect topLeft:{} bottomRight:{} size:{}]", 
		m_topLeftPos.ToString(), m_bottomRightPos.ToString(), GetSize().ToString());
}
