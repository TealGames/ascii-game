#include "pch.hpp"
#include "UITransformData.hpp"
#include "HelperFunctions.hpp"
#include "EntityData.hpp"

UITransformData::UITransformData() : UITransformData(RelativeGUIRect()) {}
UITransformData::UITransformData(const NormalizedPosition& size) : UITransformData(RelativeGUIRect(size)) {}
UITransformData::UITransformData(const RelativeGUIRect& relativeRect)
	: m_relativeRect(relativeRect), m_flags(), m_padding() {}


void UITransformData::SetFixed(const bool horizontal, const bool vertical)
{
	if (horizontal) Utils::AddFlags(m_flags, UITransformFlags::FixedHorizontal);
	else Utils::RemoveFlags(m_flags, UITransformFlags::FixedHorizontal);

	if (vertical) Utils::AddFlags(m_flags, UITransformFlags::FixedVertical);
	else Utils::RemoveFlags(m_flags, UITransformFlags::FixedVertical);
}
bool UITransformData::IsFixedVertical() const
{
	//Assert(false, std::format("Does flag vertical contain vert:{}", 
	// std::to_string(Utils::HasFlagAll(UITransformFlags::FixedHorizontal, UITransformFlags::FixedVertical))));

	return Utils::HasFlagAll(m_flags, UITransformFlags::FixedVertical);
}
bool UITransformData::IsFixedHorizontal() const
{
	return Utils::HasFlagAll(m_flags, UITransformFlags::FixedHorizontal);
}

void UITransformData::SetEventBlocker(const bool status)
{
	if (status) Utils::AddFlags(m_flags, UITransformFlags::BlockSelectionEvents);
	else Utils::RemoveFlags(m_flags, UITransformFlags::BlockSelectionEvents);
}
bool UITransformData::IsSelectionEventBlocker() const
{
	return Utils::HasFlagAll(m_flags, UITransformFlags::BlockSelectionEvents);
}

void UITransformData::SetSizeUnsafe(const Vec2& size)
{
	const Vec2 parentSize = GetSize().GetPos();
	Vec2 childSize = {};
	bool isFixedHorizontal = false;
	bool isFixedVertical = false;

	if (GetEntitySafe().GetChildCount() > 0)
	{
		for (auto& child : GetEntitySafeMutable().GetChildrenOfTypeMutable<UITransformData>())
		{
			if (child == nullptr) continue;

			isFixedHorizontal = child->IsFixedHorizontal();
			isFixedVertical = child->IsFixedVertical();
			if (!isFixedHorizontal && !isFixedVertical) continue;

			childSize = child->GetSize().GetPos();
			Vec2 newSize = childSize * Vec2(isFixedHorizontal ? parentSize.m_X / size.m_X : 1,
				isFixedVertical ? parentSize.m_Y / size.m_Y : 1);

			/*if (child->GetId() == 19) LogWarning(std::format("Setting child:{} when size set for parent:{} to newsize:{} childSize:{} og parentSize:{} new parent Size:{}",
				child->ToStringBase(), ToStringBase(), newSize.ToString(), childSize.ToString(), parentSize.ToString(), size.ToString()));*/

				//Note: by default fixed horizontal/vertical elements CANNOT have those parts modified by size,
				//but we need to update fixed children from parent, so we get around these checks by using unsafe version
			if (newSize != childSize) child->SetSizeUnsafe(newSize);
		}
	}
	m_relativeRect.SetSize(size);
	//m_OnSizeUpdated.Invoke(this);
}

void UITransformData::SetSize(const NormalizedPosition& size)
{
	const Vec2 currSize = GetSize().GetPos();
	if (IsFixedHorizontal() && size.GetX() != currSize.m_X)
	{
		Assert(false, std::format("Attempted to set the size of gui element:{} to:{} but it is fixed HORIZONTALLY "
			"so x size cannot be modified unless the HORIZONTAL lock is set to FALSE", ToString(), size.ToString()));
	}
	if (IsFixedVertical() && size.GetY() != currSize.m_Y)
	{
		Assert(false, std::format("Attempted to set the size of gui element:{} to:{} but it is fixed VERTICALLY "
			"so x size cannot be modified unless the VERTICAL lock is set to FALSE", ToString(), size.ToString()));
	}

	SetSizeUnsafe(size.GetPos());
	//LogWarning(std::format("Size target:{} set:{}", size.ToString(), m_relativeRect.ToString()));
}
void UITransformData::SetMaxSize()
{
	m_relativeRect.SetMaxSize();
	//m_OnSizeUpdated.Invoke(this);
}
void UITransformData::SetSizeX(const float sizeNormalized) { SetSize({ sizeNormalized, m_relativeRect.GetSize().GetY() }); }
void UITransformData::SetSizeY(const float sizeNormalized) { SetSize({ m_relativeRect.GetSize().GetX(), sizeNormalized }); }

void UITransformData::SetTopLeftPos(const NormalizedPosition& topLeftPos)
{
	m_relativeRect.SetTopLeft(topLeftPos);
}
void UITransformData::SetBottomRightPos(const NormalizedPosition& bottomRightPos)
{
	m_relativeRect.SetBottomRight(bottomRightPos);
}
void UITransformData::SetBounds(const NormalizedPosition& topLeftPos, const NormalizedPosition& bottomRightPos)
{
	m_relativeRect.SetBounds(topLeftPos, bottomRightPos);
}

void UITransformData::TryCenter(const bool centerX, const bool centerY)
{
	if (!centerX && !centerY) return;
	const Vec2 size = GetSize().GetPos();
	const Vec2 extraSpace = Vec2(NormalizedPosition::MAX, NormalizedPosition::MAX) - size;
	const Vec2 currTopLeft = GetRect().GetTopLeftPos().GetPos();

	SetTopLeftPos({ centerX ? extraSpace.m_X / 2 : currTopLeft.m_X, centerY ? extraSpace.m_Y / 2 + size.m_Y : currTopLeft.m_Y });
}

void UITransformData::SetPadding(const RelativeGUIPadding& padding) { m_padding = padding; }
const RelativeGUIPadding& UITransformData::GetPadding() const { return m_padding; }
RelativeGUIPadding& UITransformData::GetPaddingMutable() { return m_padding; }

NormalizedPosition UITransformData::GetSize() const { return m_relativeRect.GetSize(); }
const RelativeGUIRect& UITransformData::GetRect() const { return m_relativeRect; }
RelativeGUIRect& UITransformData::GetRectMutable() { return m_relativeRect; }

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
	return std::format("[Id:{} Type:{} TL:{} BR:{} Size:{}]", GetEntitySafe().ToStringId(),
		Utils::FormatTypeName(typeid(*this).name()), m_relativeRect.GetTopLeftPos().ToString(),
		m_relativeRect.GetBottomRighttPos().ToString(), GetSize().ToString());
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
