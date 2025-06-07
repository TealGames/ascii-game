#include "pch.hpp"
#include "UITransformSystem.hpp"

namespace ECS
{
	UITransformSystem::UITransformSystem() {}

	GUIRect UITransformSystem::CalculateRect(const UITransformData& transform, const GUIRect& parentInfo) const
	{
		return GUIRect(parentInfo.m_TopLeftPos + GetSizeFromFactor(Abs(transform.GetRect().GetTopLeftPos().GetPos() -
			NormalizedPosition::TOP_LEFT), parentInfo.GetSize()), transform.GetRect().GetSize(parentInfo.GetSize()));
	}
	GUIRect UITransformSystem::CalculateChildRect(const UITransformData& transform, const GUIRect& thisRenderInfo) const
	{
		const RelativeGUIPadding& padding = transform.GetPadding();
		if (!padding.HasNonZeroPadding()) return thisRenderInfo;

		const ScreenPosition paddingTopLeft = ScreenPosition(padding.m_Left.GetValue() * thisRenderInfo.GetSize().m_X,
			padding.m_Top.GetValue() * thisRenderInfo.GetSize().m_Y);
		const ScreenPosition paddingBottomRight = ScreenPosition(padding.m_Right.GetValue() * thisRenderInfo.GetSize().m_X,
			padding.m_Bottom.GetValue() * thisRenderInfo.GetSize().m_Y);
		//if (padding.HasNonZeroPadding())
		//{
		//	//Assert(false, std::format("NOn zero padding tTL:{} bR:{} padding:{}", paddingTopLeft.ToString(), paddingBottomRight.ToString(), m_padding.ToString()));
		//}

		return GUIRect(thisRenderInfo.m_TopLeftPos + paddingTopLeft, thisRenderInfo.GetSize() - paddingTopLeft - paddingBottomRight);
	}
}
