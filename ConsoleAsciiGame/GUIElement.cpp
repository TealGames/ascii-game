#include "pch.hpp"
#include "GUIElement.hpp"
#include "Debug.hpp"
#include "HelperFunctions.hpp"

static constexpr bool DRAW_RENDER_BOUNDS = true;

GUIElement::GUIElement() : GUIElement(RelativeGUIRect()) {}
GUIElement::GUIElement(const NormalizedPosition& size) : GUIElement(RelativeGUIRect(size)) {}
GUIElement::GUIElement(const RelativeGUIRect& relativeRect) 
	: m_children(), m_parent(nullptr), m_id(GenerateId()), m_relativeRect(relativeRect), m_OnFarthestChildElementAttached(), m_flags() {}

GUIElementID GUIElement::GenerateId()
{
	static GUIElementID id = 0;
	if (id == std::numeric_limits< GUIElementID>::max())
	{
		LogError(std::format("Attempted to generate next gui element id but reached max value"));
		return -1;
	}
	return id++;
}
GUIElementID GUIElement::GetId() const { return m_id; };

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

	if (!m_children.empty())
	{
		for (auto& child : m_children)
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
			"so x size cannot be modified unless the HORIZONTAL lock is set to FALSE", ToStringBase(), size.ToString()));
	}
	if (IsFixedVertical() && size.GetY() != currSize.m_Y)
	{
		Assert(false, std::format("Attempted to set the size of gui element:{} to:{} but it is fixed VERTICALLY "
			"so x size cannot be modified unless the VERTICAL lock is set to FALSE", ToStringBase(), size.ToString()));
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
const RelativeGUIPadding& GUIElement::GetPadding()
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

GUIElement* GUIElement::GetParentMutable()
{
	return m_parent;
}
const GUIElement* GUIElement::GetParent() const
{
	return m_parent;
}

size_t GUIElement::PushChild(GUIElement* element)
{
	if (element == nullptr)
	{
		Assert(false, std::format("Attempted to push NULL child to element:{}", ToStringBase()));
		return 0;
	}

	GUIElement* createdElement= m_children.emplace_back(element);
	createdElement->m_parent = this;
	//Note: we store index before event in case event triggers additional
	//children creation that might ruin the result of this function
	const size_t childIndex = GetChildCount() - 1;

	GUIElement* currEl = this;
	while (currEl->m_parent != nullptr)
	{
		currEl = currEl->m_parent;
	}
	currEl->m_OnFarthestChildElementAttached.Invoke(this);
	return childIndex;
}
GUIElement* GUIElement::TryPopChildAt(const size_t index)
{
	if (index >= m_children.size()) return nullptr;

	GUIElement* popped = m_children[index];
	m_children.erase(m_children.begin() + index);
	return popped;
}
std::vector<GUIElement*> GUIElement::TryPopChildren(const size_t& startIndex, const size_t& count)
{
	std::vector<GUIElement*> poppedChildren = {};
	for (size_t i = std::min(startIndex + count, m_children.size())-1; i>= startIndex; i--)
	{
		poppedChildren.push_back(TryPopChildAt(i));
	}
	return poppedChildren;
}
std::vector<GUIElement*> GUIElement::PopAllChildren()
{
	std::vector<GUIElement*> childrenCopy = m_children;
	m_children.clear();
	return childrenCopy;
}

std::vector<GUIElement*>& GUIElement::GetChildrenMutable() { return m_children; }
const std::vector<GUIElement*>& GUIElement::GetChildren() const { return m_children; }

GUIElement* GUIElement::TryGetChildAtMutable(const size_t index)
{
	if (index >= m_children.size()) return nullptr;
	return m_children[index];
}
GUIElement* GUIElement::TryGetChildMutable(const GUIElementID id)
{
	if (m_children.empty()) return nullptr;

	//Note: we go backwards in case we decide later we want to remove invalid children
	for (int i=m_children.size()-1; i>=0; i--)
	{
		if (m_children[i] == nullptr) continue;
		if (m_children[i]->GetId() == id)
			return m_children[i];
	}
	return nullptr;
}
GUIElement* GUIElement::FindRecursiveMutable(const GUIElementID id)
{
	if (m_id == id) return this;
	if (m_children.empty()) return nullptr;

	GUIElement* foundElement = nullptr;
	for (auto& child : m_children)
	{
		if (child == nullptr) continue;

		foundElement = FindRecursiveMutable(id);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}
GUIElement* GUIElement::FindParentRecursiveMutable(const GUIElementID id, size_t* childIndex)
{
	if (m_children.empty()) return nullptr;

	GUIElement* foundElement = nullptr;
	for (size_t i=0; i<m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		if (m_children[i]->m_id == id)
		{
			if (childIndex != nullptr) *childIndex = i;
			return this;
		}
		foundElement= FindParentRecursiveMutable(id, childIndex);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}
size_t GUIElement::GetChildCount() const { return m_children.size(); }

RenderInfo GUIElement::CalculateRenderInfo(const RenderInfo& parentInfo) const
{
	return RenderInfo(parentInfo.m_TopLeftPos + GetSizeFromFactor(Abs(m_relativeRect.GetTopLeftPos().GetPos() -
		NormalizedPosition::TOP_LEFT), parentInfo.m_RenderSize), m_relativeRect.GetSize(parentInfo.m_RenderSize));
}
RenderInfo GUIElement::CalculateChildRenderInfo(const RenderInfo& thisRenderInfo) const
{
	if (!m_padding.HasNonZeroPadding()) return thisRenderInfo;

	const ScreenPosition paddingTopLeft = ScreenPosition(m_padding.m_Left.GetValue() * thisRenderInfo.m_RenderSize.m_X, 
														 m_padding.m_Top.GetValue() * thisRenderInfo.m_RenderSize.m_Y);
	const ScreenPosition paddingBottomRight = ScreenPosition(m_padding.m_Right.GetValue() * thisRenderInfo.m_RenderSize.m_X, 
															 m_padding.m_Bottom.GetValue() * thisRenderInfo.m_RenderSize.m_Y);
	if (m_padding.HasNonZeroPadding())
	{
		//Assert(false, std::format("NOn zero padding tTL:{} bR:{} padding:{}", paddingTopLeft.ToString(), paddingBottomRight.ToString(), m_padding.ToString()));
	}

	return RenderInfo(thisRenderInfo.m_TopLeftPos + paddingTopLeft, thisRenderInfo.m_RenderSize- paddingTopLeft - paddingBottomRight);
}

void GUIElement::UpdateRecursive(const float deltaTime)
{
	//If we have invalid rect, we set to max size to ensure rendering works
	const Vec2 size = m_relativeRect.GetSize().GetPos();
	if (Utils::ApproximateEqualsF(size.m_X, 0) || Utils::ApproximateEqualsF(size.m_Y, 0))
	{
		RelativeGUIRect before = m_relativeRect;
		//Assert(false, std::format("Tried to set panel:{} of size:{} to max", std::to_string(m_id), m_relativeRect.GetSize().ToString()));
		m_relativeRect.SetMaxSize();
		//LogWarning(std::format("Changed x for rect of id:{} to:{} -> {}", std::to_string(m_id), before.ToString(), m_relativeRect.ToString()));
	}
	 
	Update(deltaTime);

	if (m_children.empty()) return;
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		m_children[i]->UpdateRecursive(deltaTime);
	}
}

void GUIElement::RenderRecursive(const RenderInfo& parentInfo)
{
	//We get this render info based on this relative pos
	const RenderInfo thisRenderInfo = CalculateRenderInfo(parentInfo);
	m_lastRenderInfo = GUIRect(thisRenderInfo.m_TopLeftPos, thisRenderInfo.m_RenderSize);

	if (DRAW_RENDER_BOUNDS) DrawRectangleLines(thisRenderInfo.m_TopLeftPos.m_X, thisRenderInfo.m_TopLeftPos.m_Y, 
		thisRenderInfo.m_RenderSize.m_X, thisRenderInfo.m_RenderSize.m_Y, YELLOW);

	//LogWarning(std::format("Rendering id:{} at:{}", std::to_string(m_id), thisRenderInfo.ToString()));
	Render(thisRenderInfo);
	/*LogError(std::format("Rendering element calculated from parent info:{} rect:{} current info:{}", 
		renderInfo.ToString(), m_relativeRect.ToString(), thisRenderInfo.ToString()));*/

	if (m_children.empty()) return;

	const RenderInfo childRenderInfo = CalculateChildRenderInfo(thisRenderInfo);
	/*if (m_padding.HasNonZeroPadding())
	{
		Assert(false, std::format("Padding:{} this render:{} child render:{}", 
			m_padding.ToString(), thisRenderInfo.ToString(), childRenderInfo.ToString()));
	}*/

	for (size_t i=0; i<m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		//Note: we assume space reserved for this element is the space it actually 
		//used for render (and thus can be used for children) because we do not want size
		//changes during render especially becuase of normalized vs in-game space coordinates
		m_children[i]->RenderRecursive(childRenderInfo);
	}
}
const GUIRect& GUIElement::GetLastFrameRect() const
{
	return m_lastRenderInfo;
}

std::string GUIElement::ToStringBase() const
{
	return std::format("[Id:{} Type:{} TL:{} BR:{} Size:{}]", std::to_string(m_id), 
		Utils::FormatTypeName(typeid(*this).name()), m_relativeRect.GetTopLeftPos().ToString(), 
		m_relativeRect.GetBottomRighttPos().ToString(), GetSize().ToString());
}

std::string GUIElement::ToStringRecursive(std::string startNewLine) const
{
	std::string result = "";
	result += std::format("\n{}-> {}", startNewLine, ToStringBase());

	if (m_children.empty()) return result;

	startNewLine += "    ";
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		result += m_children[i]->ToStringRecursive(startNewLine);
	}

	return result;
}
//RenderInfo GUIElement::CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const
//{
//	return parentInfo;
//}