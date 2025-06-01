#include "pch.hpp"
#include "GUIElement.hpp"
#include "Debug.hpp"
#include "HelperFunctions.hpp"
#include "EntityData.hpp"

GUIElement::GUIElement() : GUIElement(RelativeGUIRect()) {}
GUIElement::GUIElement(const NormalizedPosition& size) : GUIElement(RelativeGUIRect(size)) {}
GUIElement::GUIElement(const RelativeGUIRect& relativeRect)
	: m_relativeRect(relativeRect), m_flags(), m_lastRenderInfo(), m_padding() {}


void GUIElement::SetFixed(const bool horizontal, const bool vertical) 
{ 
	if (horizontal) Utils::AddFlags(m_flags, GUIElementFlags::FixedHorizontal);
	else Utils::RemoveFlags(m_flags, GUIElementFlags::FixedHorizontal);

	if (vertical) Utils::AddFlags(m_flags, GUIElementFlags::FixedVertical);
	else Utils::RemoveFlags(m_flags, GUIElementFlags::FixedVertical);
}
bool GUIElement::IsFixedVertical() const
{
	//Assert(false, std::format("Does flag vertical contain vert:{}", 
	// std::to_string(Utils::HasFlagAll(GUIElementFlags::FixedHorizontal, GUIElementFlags::FixedVertical))));

	return Utils::HasFlagAll(m_flags, GUIElementFlags::FixedVertical);
}
bool GUIElement::IsFixedHorizontal() const
{
	return Utils::HasFlagAll(m_flags, GUIElementFlags::FixedHorizontal);
}

void GUIElement::SetEventBlocker(const bool status)
{
	if (status) Utils::AddFlags(m_flags, GUIElementFlags::BlockSelectionEvents);
	else Utils::RemoveFlags(m_flags, GUIElementFlags::BlockSelectionEvents);
}
bool GUIElement::IsSelectionEventBlocker() const
{
	return Utils::HasFlagAll(m_flags, GUIElementFlags::BlockSelectionEvents);
}

void GUIElement::SetSizeUnsafe(const Vec2& size)
{
	const Vec2 parentSize = GetSize().GetPos();
	Vec2 childSize = {};
	bool isFixedHorizontal = false;
	bool isFixedVertical = false;

	if (GetEntitySafe().GetChildCount() > 0)
	{
		for (auto& child : GetEntitySafeMutable().GetChildrenOfTypeMutable<GUIElement>())
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

void GUIElement::SetSize(const NormalizedPosition& size) 
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
void GUIElement::SetMaxSize()
{
	m_relativeRect.SetMaxSize();
	//m_OnSizeUpdated.Invoke(this);
}
void GUIElement::SetSizeX(const float sizeNormalized) { SetSize({ sizeNormalized, m_relativeRect.GetSize().GetY() }); }
void GUIElement::SetSizeY(const float sizeNormalized) { SetSize({ m_relativeRect.GetSize().GetX(), sizeNormalized}); }

void GUIElement::SetTopLeftPos(const NormalizedPosition& topLeftPos)
{
	m_relativeRect.SetTopLeft(topLeftPos);
}
void GUIElement::SetBottomRightPos(const NormalizedPosition& bottomRightPos)
{
	m_relativeRect.SetBottomRight(bottomRightPos);
}
void GUIElement::SetBounds(const NormalizedPosition& topLeftPos, const NormalizedPosition& bottomRightPos)
{
	m_relativeRect.SetBounds(topLeftPos, bottomRightPos);
}

void GUIElement::TryCenter(const bool centerX, const bool centerY)
{
	if (!centerX && !centerY) return;
	const Vec2 size= GetSize().GetPos();
	const Vec2 extraSpace = Vec2(NormalizedPosition::MAX, NormalizedPosition::MAX) - size;
	const Vec2 currTopLeft = GetRect().GetTopLeftPos().GetPos();

	SetTopLeftPos({ centerX? extraSpace.m_X / 2 : currTopLeft.m_X, centerY? extraSpace.m_Y/2 + size.m_Y: currTopLeft.m_Y});
}

void GUIElement::SetPadding(const RelativeGUIPadding& padding)
{
	m_padding = padding;
}
const RelativeGUIPadding& GUIElement::GetPadding() const
{
	return m_padding;
}
RelativeGUIPadding& GUIElement::GetPaddingMutable()
{
	return m_padding;
}

NormalizedPosition GUIElement::GetSize() const
{
	return m_relativeRect.GetSize();
}
const RelativeGUIRect& GUIElement::GetRect() const
{
	return m_relativeRect;
}
RelativeGUIRect& GUIElement::GetRectMutable()
{
	return m_relativeRect;
}

void GUIElement::SetLastFrameRect(const GUIRect& rect)
{
	m_lastRenderInfo = rect;
}

const GUIRect& GUIElement::GetLastFrameRect() const
{
	return m_lastRenderInfo;
}

//std::vector<std::string> GUIElement::GetDependencyFlags() const
//{
//	return {};
//}
void GUIElement::InitFields()
{
	m_Fields = {};
}

std::string GUIElement::ToString() const
{
	return std::format("[Id:{} Type:{} TL:{} BR:{} Size:{}]", GetEntitySafe().ToStringId(),
		Utils::FormatTypeName(typeid(*this).name()), m_relativeRect.GetTopLeftPos().ToString(),
		m_relativeRect.GetBottomRighttPos().ToString(), GetSize().ToString());
}

void GUIElement::Deserialize(const Json& json)
{
	//TODO: implmenet
	return;
}
Json GUIElement::Serialize()
{
	//TOOO: implement
	return {};
}
//RenderInfo GUIElement::CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const
//{
//	return parentInfo;
//}