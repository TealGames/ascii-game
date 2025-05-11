#include "pch.hpp"
#include "GUIElement.hpp"
#include "Debug.hpp"

GUIElement::GUIElement() : GUIElement(NormalizedPosition()) {}
GUIElement::GUIElement(const NormalizedPosition& size) : GUIElement(RelativeGUIRect({}, size)) {}
GUIElement::GUIElement(const RelativeGUIRect& relativeRect) 
	: m_children(), m_id(GenerateId()), m_relativeRect(relativeRect) {}

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

void GUIElement::SetSize(const NormalizedPosition& size)
{
	m_relativeRect.SetSize(size, true);
}
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

void GUIElement::PushChild(GUIElement* element)
{
	m_children.emplace_back(element);
}
GUIElement* GUIElement::TryPopChildAt(const size_t index)
{
	if (index >= m_children.size()) return nullptr;

	GUIElement* popped = m_children[index];
	m_children.erase(m_children.begin() + index);
	return popped;
}
std::vector<GUIElement*> GUIElement::PopAllChildren()
{
	std::vector<GUIElement*> childrenCopy = m_children;
	m_children.clear();
	return childrenCopy;
}

std::vector<GUIElement*>& GUIElement::GetChildrenMutable() { return m_children; }
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

void GUIElement::UpdateRecursive(const float deltaTime)
{
	Update(deltaTime);

	if (m_children.empty()) return;
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		m_children[i]->UpdateRecursive(deltaTime);
	}
}

void GUIElement::RenderRecursive(const RenderInfo& renderInfo)
{
	//If we have invalid rect, we set to max size
	if (m_relativeRect.GetSize().IsZero()) m_relativeRect.SetMaxSize();

	//We get this render info based on this relative pos
	const RenderInfo thisRenderInfo = RenderInfo(renderInfo.m_TopLeftPos + GetRealPos(m_relativeRect.GetTopLeftPos() - 
		NormalizedPosition::TOP_LEFT, renderInfo.m_RenderSize), m_relativeRect.GetSize(renderInfo.m_RenderSize));
	Render(thisRenderInfo);
	LogError(std::format("Rendered with info:{}", thisRenderInfo.ToString()));

	if (m_children.empty()) return;
	for (size_t i=0; i<m_children.size(); i++)
	{
		if (m_children[i] == nullptr) continue;
		//Note: we assume space reserved for this element is the space it actually 
		//used for render (and thus can be used for children) because we do not want size
		//changes during render especially becuase of normalized vs in-game space coordinates
		m_children[i]->RenderRecursive(thisRenderInfo);
	}
}
//RenderInfo GUIElement::CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const
//{
//	return parentInfo;
//}