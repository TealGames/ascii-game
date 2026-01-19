#include "pch.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include "Utils/Math/MathAdvanced.hpp"

UITransformData::UITransformData() : UITransformData(UIRect()) {}
UITransformData::UITransformData(const NormalizedPos& size) : UITransformData(UIRect(size)) {}
UITransformData::UITransformData(const UIRect& relativeRect)
	: m_localRect(relativeRect), m_flags(UITransformFlags::None), m_Padding(), 
	m_lastGlobalScreenRect() {}

void UITransformData::SetFixed(const bool horizontal, const bool vertical)
{
	if (horizontal) Utils::AddFlags(m_flags, UITransformFlags::FixedHorizontal);
	else Utils::RemoveFlags(m_flags, UITransformFlags::FixedHorizontal);

	if (vertical) Utils::AddFlags(m_flags, UITransformFlags::FixedVertical);
	else Utils::RemoveFlags(m_flags, UITransformFlags::FixedVertical);
}
bool UITransformData::IsFixedVertical() const { return Utils::HasFlagAll(m_flags, UITransformFlags::FixedVertical); }
bool UITransformData::IsFixedHorizontal() const { return Utils::HasFlagAll(m_flags, UITransformFlags::FixedHorizontal); }

void UITransformData::SetLastGlobalScreenRect(const UIRect& area)
{
	m_lastGlobalScreenRect = area;
}
const UIRect& UITransformData::GetLastGlobalScreenRect() const
{
	return m_lastGlobalScreenRect;
}

void UITransformData::SetEventBlocker(const bool status)
{
	if (status) Utils::AddFlags(m_flags, UITransformFlags::EventBlocker);
	else Utils::RemoveFlags(m_flags, UITransformFlags::EventBlocker);
}
bool UITransformData::IsSelectionEventBlocker() const
{
	return Utils::HasFlagAll(m_flags, UITransformFlags::EventBlocker);
}

void UITransformData::UpdateFixedChildren(const Vec2& oldParentSize)
{
	const Vec2 newParentSize = GetLocalSize().AsVec2();
	Vec2 childSize = {};
	bool isFixedHorizontal = false;
	bool isFixedVertical = false;

	if (GetEntity().GetChildCount() > 0)
	{
		for (auto& child : GetEntityMutable().GetChildrenOfTypeMutable<UITransformData>())
		{
			if (child == nullptr) continue;

			isFixedHorizontal = child->IsFixedHorizontal();
			isFixedVertical = child->IsFixedVertical();
			if (!isFixedHorizontal && !isFixedVertical) continue;

			childSize = child->GetLocalSize().AsVec2();
			Vec2 newSize = childSize * Vec2(isFixedHorizontal ? newParentSize.m_X / oldParentSize.m_X : 1,
				isFixedVertical ? newParentSize.m_Y / oldParentSize.m_Y : 1);

			/*if (child->GetId() == 19) LogWarning(std::format("Setting child:{} when size set for parent:{} to newsize:{} childSize:{} og parentSize:{} new parent Size:{}",
				child->ToStringBase(), ToStringBase(), newSize.ToString(), childSize.ToString(), parentSize.ToString(), size.ToString()));*/

			//Note: by default fixed horizontal/vertical elements CANNOT have those parts modified by size,
			//but we need to update fixed children from parent, so we get around these checks by using unsafe version
			if (newSize != childSize) child->SetSizeUnsafe(newSize);
		}
	}
}
void UITransformData::SetSizeUnsafe(const Vec2& size)
{
	const Vec2 oldParentSize = GetLocalSize().AsVec2();
	m_localRect.SetSize(size);
	UpdateFixedChildren(oldParentSize);
	//m_OnSizeUpdated.Invoke(this);
}

bool UITransformData::DoLocksAllowSizeChange(const NormalizedPos& proposedNewSize) const
{
	const Vec2 currSize = GetLocalSize().AsVec2();
	if (IsFixedHorizontal() && !Utils::ApproximateEqualsF(proposedNewSize.GetX(), currSize.m_X))
		return false;

	if (IsFixedVertical() && !Utils::ApproximateEqualsF(proposedNewSize.GetY(), currSize.m_Y))
		return false;

	return true;
}

void UITransformData::SetLocalSize(const NormalizedPos& newSize)
{
	if (!DoLocksAllowSizeChange(newSize))
	{
		LogError(std::format("Attempted to set the size of UI TRANSFORM:{} to:{} but one or more locks "
			"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
			"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
		return;
	}

	SetSizeUnsafe(newSize.AsVec2());
}
void UITransformData::SetMaxRelativeSize() { SetLocalSize(NormalizedPos(NormalizedValue::MAX, NormalizedValue::MAX)); }
void UITransformData::SetRelativeSizeX(const float sizeNormalized) { SetLocalSize({ sizeNormalized, m_localRect.GetSize().GetY() }); }
void UITransformData::SetRelativeSizeY(const float sizeNormalized) { SetLocalSize({ m_localRect.GetSize().GetX(), sizeNormalized }); }

//NOTE: for setting the positions since we do true for maintaining size, messing up fixed vertical/horizontal is not a problem
void UITransformData::SetLocalTopLeftPos(const NormalizedPos& topLeftPos) { m_localRect.SetTopLeft(topLeftPos, true); }
void UITransformData::SetLocalTopRightPos(const NormalizedPos& topRightPos) { m_localRect.SetTopRight(topRightPos, true); }
void UITransformData::SetLocalBottomRightPos(const NormalizedPos& bottomRightPos) { m_localRect.SetBottomRight(bottomRightPos, true); }
void UITransformData::SetLocalBottomLeftPos(const NormalizedPos& bottomLeftPos) { m_localRect.SetBottomLeft(bottomLeftPos, true); }

void UITransformData::SetLocalBoundsBLTR(const NormalizedPos& bottomLeftPos, const NormalizedPos& topRightPos)
{
	const NormalizedPos newSize = topRightPos - bottomLeftPos;
	if (!DoLocksAllowSizeChange(newSize))
	{
		LogError(std::format("Attempted to set bounds of UI TRANSFORM:{} to BottomLeft:{} TopRight:{} (newSize:{}) but one or more locks "
			"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
			"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), bottomLeftPos.ToString(), topRightPos.ToString(),
			newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
		return;
	}

	const Vec2 oldSize = m_localRect.GetSize().AsVec2();
	m_localRect.SetBoundsBLTR(bottomLeftPos, topRightPos);
	UpdateFixedChildren(oldSize);
}
void UITransformData::SetLocalBoundsTLBR(const NormalizedPos& topLeftPos, const NormalizedPos& bottomRightPos)
{
	const NormalizedPos newSize = NormalizedPos(bottomRightPos.m_X - topLeftPos.m_X, topLeftPos.m_Y - bottomRightPos.m_Y);
	if (!DoLocksAllowSizeChange(newSize))
	{
		LogError(std::format("Attempted to set bounds of UI TRANSFORM:{} to TopLeft:{} BottomRight:{} (newSize:{}) but one or more locks "
			"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
			"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), topLeftPos.ToString(), bottomRightPos.ToString(),
			newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
		return;
	}

	const Vec2 oldSize = m_localRect.GetSize().AsVec2();
	m_localRect.SetBoundsTLBR(topLeftPos, bottomRightPos);
	UpdateFixedChildren(oldSize);
}

void UITransformData::CenterWithinParent(const bool centerX, const bool centerY)
{
	if (!centerX && !centerY) 
		return;

	const Vec2 size = GetLocalSize().AsVec2();
	const Vec2 emptySpaceSize = Vec2(NormalizedValue::MAX, NormalizedValue::MAX) - size;
	const NormalizedPos currBottomLeft = GetLocalRect().GetBottomLeftPos();

	SetLocalBottomLeftPos(currBottomLeft + NormalizedPos{ centerX ? emptySpaceSize.m_X / 2 : 0.0f,
														  centerY ? emptySpaceSize.m_Y / 2 : 0.0f});
}

NormalizedPos UITransformData::GetLocalSize() const { return m_localRect.GetSize(); }
const UIRect& UITransformData::GetLocalRect() const { return m_localRect; }

UIRect UITransformData::CalculateWorldRect(const UIRect& parentGlobalRect) const
{
	return ::CalculateWorldRect(parentGlobalRect, m_localRect);
}
UIRect UITransformData::CalculateChildParentRect(const UIRect& thisGlobalRect) const
{
	if (m_Padding.HasNoPadding()) 
		return thisGlobalRect;

	const NormalizedPos topLeftPadding = NormalizedPos(m_Padding.m_Left, m_Padding.m_Top);
	return UIRect(m_localRect.GetTopLeftPos() - topLeftPadding,
			      m_localRect.GetSize() - topLeftPadding - NormalizedPos(m_Padding.m_Right, m_Padding.m_Bottom));
}

Mat3 UITransformData::CalculateLocalModelMatrix() const
{
	return Utils::CalculateUIModelMatrix(nullptr, m_localRect.GetTopLeftPos().AsVec2(), 
		m_localRect.GetSize().AsVec2(), m_localRect.m_Pivot.AsVec2());
}
Mat3 UITransformData::CalculateWorldModelMatrix() const
{
	Mat3 globalMatrix = CalculateLocalModelMatrix();

	const EntityData* parent = GetEntity().GetParent();
	if (parent != nullptr)
	{
		//NOTE: it is a requirement for all ui elements to be an ui context ONLY so parent should have a transform
		globalMatrix = globalMatrix * parent->TryGetComponent<UITransformData>()->CalculateLocalModelMatrix();
		parent = parent->GetParent();
	}
	return globalMatrix;
}

//std::vector<std::string> UITransformData::GetDependencyFlags() const
//{
//	return {};
//}
void UITransformData::InitFields()
{
	m_Fields = {};
}

std::string UITransformData::ToString() const
{
	return std::format("[Id:{} BL:{} TR:{} Size:{} LAST WA:{}]", GetEntity().ToStringId(),
		m_localRect.GetBottomLeftPos().ToString(), m_localRect.GetBottomRightPos().ToString(), 
		GetLocalSize().ToString(), m_lastGlobalScreenRect.ToString());
}

void UITransformData::Deserialize(const Json& json)
{
	//TODO: implmenet
	return;
}
Json UITransformData::Serialize()
{
	//TOOO: implement
	return {};
}
//RenderInfo UITransformData::CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const
//{
//	return parentInfo;
//}
