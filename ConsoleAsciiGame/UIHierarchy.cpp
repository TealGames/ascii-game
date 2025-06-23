#include "pch.hpp"
#include "UIHierarchy.hpp"
#include "Debug.hpp"
#include <queue>
#include <vector>
#include "GlobalEntityManager.hpp"
//#include "EntityData.hpp"

static constexpr bool DRAW_RENDER_BOUNDS = false;

UIHierarchy::UIHierarchy(GlobalEntityManager& globalEntityManager, const Vec2Int rootCanvasSize)
	: m_uiRoot(nullptr), m_rootSize(rootCanvasSize), m_layerRoots({}), m_OnElementAdded(), m_OnElementRemoved()
{
	//m_rootElements.reserve(TOP_LAYER);
	EntityData& uiRootEntity= globalEntityManager.CreateGlobalEntity(ROOT_UI_ENTITY_NAME, TransformData());
	UITransformData& rootTransform = uiRootEntity.AddComponent<UITransformData>();
	LogWarning(std::format("Root transform: {}", rootTransform.ToString()));
	m_uiRoot = &rootTransform;

	if (m_uiRoot == nullptr)
	{
		Assert(false, std::format("Attmpted to create the ui root element "
			"but its ui transform element was null. Entity:{}", uiRootEntity.ToString()));
	}
	//m_rootElements.reserve(MAX_LAYERS);
}

bool UIHierarchy::IsValidLayer(const UILayer layer, const bool logError) const
{
	const bool isValid= layer <= TOP_LAYER;
	if (logError && !isValid)
	{
		Assert(this, false, std::format("Attempted to use a gui layer:{} "
			"but the underlying layer value is invalid", std::to_string(layer)));
	}
	return isValid;
}

UITransformData* UIHierarchy::CreateNewLayer(const UILayer layer)
{
	if (!IsValidLayer(layer, true)) return nullptr;
	if (m_layerRoots[layer]!=nullptr)
	{
		Assert(false, std::format("Attempted to create a new layer:{} in gui hierarchy "
			"but that layer has already been created", std::to_string(layer)));
		return nullptr;
	}

	EntityData& layerRoot= m_uiRoot->GetEntityMutable().CreateChild(std::format("Layer{}", std::to_string(layer)), TransformData());
	UITransformData& rootElement = layerRoot.AddComponent<UITransformData>();
	m_layerRoots[layer] = &rootElement;

	layerRoot.m_OnFarthestChildElementAttached.AddListener([this](EntityData* entity)-> void
		{
			if (entity == nullptr)
			{
				Assert(false, std::format("Invoked new gui element added with null element"));
				return;
			}

			//Note: gui elements require their placement only in other gui elements, so we can guarantee they are gui elements
			m_OnElementAdded.Invoke(entity->TryGetComponentMutable<UITransformData>());
		});

	return &rootElement;
}
bool UIHierarchy::IsLayerRootID(const ECS::EntityID id) const
{
	//TODO: we could speed up this search if we reserve each layer with its own range
	
	//NOTE: we could go through the root of each layer OR we can just look at cached
	//root elements, but we do root elements since they are faster to access
	if (m_uiRoot->GetEntity().GetChildCount()==0) return false;

	for (const auto& rootElement : m_layerRoots)
	{
		if (rootElement->GetEntity().GetId() == id)
			return true;
	}
	return false;
}

UITransformData* UIHierarchy::FindMutable(const ECS::EntityID id)
{
	//TODO: there could be a speed up by having each layer be reserved some amounts of ids so we dont have to search all layers
	UITransformData* foundElement = nullptr;
	for (size_t i=0; i<m_layerRoots.size(); i++)
	{
		if (m_layerRoots[i] == nullptr)
		{
			Assert(false, std::format("Tried to find gui element with id:{} MUTABLE"
				"but encountered lyaer root:{} that is null", EntityData::ToString(id), std::to_string(i)));
			return nullptr;
		}

		foundElement = m_layerRoots[i]->GetEntityMutable().FindComponentRecursiveMutable<UITransformData>(id);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}
UITransformData* UIHierarchy::FindParentMutable(const ECS::EntityID id, size_t* foundChildIndex)
{
	UITransformData* foundEntity= nullptr;
	for (size_t i = 0; i < m_layerRoots.size(); i++)
	{
		if (m_layerRoots[i] == nullptr)
		{
			Assert(false, std::format("Tried to find gui element parent with id:{} MUTABLE"
				"but encountered lyaer root:{} that is null", EntityData::ToString(id), std::to_string(i)));
			return nullptr;
		}

		foundEntity = m_layerRoots[i]->GetEntityMutable().FindParentComponentRecursiveMutable<UITransformData>(id, foundChildIndex);
		if (foundEntity != nullptr) return foundEntity;
	}
	return nullptr;
}

std::vector<UITransformData*> UIHierarchy::GetLayerRootsMutable(const bool topLayerFirst)
{
	if (m_layerRoots.empty()) return {};
	std::vector<UITransformData*> layers = {};


	if (topLayerFirst)
	{
		for (int i = m_layerRoots.size() - 1; i >= 0; i--)
			layers.push_back(m_layerRoots[i]);
	}
	else
	{
		for (auto& layer : m_layerRoots)
			layers.push_back(layer);
	}
	return layers;
}

const Vec2Int UIHierarchy::GetRootSize() const { return m_rootSize; }
const UITransformData* UIHierarchy::GetRootElement() const { return m_uiRoot; }
const UITransformData* UIHierarchy::GetLayerRootElement(const UILayer layer) const
{
	if (!IsValidLayer(layer, true)) return nullptr;
	return m_layerRoots[layer];
}

std::tuple<EntityData*, UITransformData*> UIHierarchy::CreateAtRoot(const UILayer layer, const std::string& name)
{
	if (!IsValidLayer(layer, true)) return std::make_tuple<EntityData*, UITransformData*>(nullptr, nullptr);
	if (m_layerRoots[layer] == nullptr) CreateNewLayer(layer);
	return m_layerRoots[layer]->GetEntityMutable().CreateChildUI(name);
}

//void GUIHierarchy::SetRoot(const GUILayer layer, GUIElement* root)
//{
//	if (root == nullptr) return;
//	auto rootIt = m_layerRoots.find(layer);
//
//	if (rootIt == m_layerRoots.end()) rootIt = CreateNewLayer(layer);
//	rootIt->second = root;
//}
void UIHierarchy::AddToRoot(const UILayer layer, UITransformData* element)
{
	if (!IsValidLayer(layer, true)) return;
	//Note: we could use the existing functions to accomplish this, but we can make assumptions
	//can speed up some of the process here
	if (element == nullptr)
	{
		Assert(this, false, std::format("Tried to add element to root of gui hierarchy but element is NULL"));
		return;
	}

	if (m_layerRoots[layer]==nullptr) CreateNewLayer(layer);

	m_layerRoots[layer]->GetEntityMutable().PushChild(element->GetEntityMutable());
	//m_OnElementAdded.Invoke(element);
}
UITransformData* UIHierarchy::RemoveFromRoot(const UILayer layer, const ECS::EntityID id)
{
	if (IsLayerRootID(id))
	{
		Assert(this, false, std::format("Tried to remove element with id:'{}' from hierarchy, "
			"but it is a layer root element and those cannot be removed", EntityData::ToString(id)));
		return nullptr;
	}
	if (m_layerRoots[layer] == nullptr)
	{
		Assert(this, false, std::format("Attempted to remove element from layer:{} "
			"root with id:{} but that layer has no root", std::to_string(layer), EntityData::ToString(id)));
		return nullptr;
	}

	return m_layerRoots[layer]->GetEntityMutable().TryPopChildAs<UITransformData>(id);
}
//bool GUIHierarchy::TryAddElementAsChild(const ECS::EntityID parentId, GUIElement& element)
//{
//	GUIElement* foundElement = FindMutable(parentId);
//	if (foundElement == nullptr) return false;
//
//	foundElement->GetEntitySafeMutable().PushChild(element.GetEntitySafeMutable());
//	//m_OnElementAdded.Invoke(element);
//	return true;
//}
//GUIElement* GUIHierarchy::TryRemoveElementChildren(const ECS::EntityID id)
//{
//	GUIElement* foundElement = FindMutable(id);
//	if (foundElement == nullptr) return nullptr;
//
//	GUIElement* childAsEleemnt = nullptr;
//	for (auto& child : foundElement->GetEntitySafeMutable().PopAllChildren())
//	{
//		childAsEleemnt = child->TryGetComponentMutable<GUIElement>();
//		if (childAsEleemnt == nullptr) continue;
//
//		m_OnElementRemoved.Invoke(childAsEleemnt);
//	}
//
//	return foundElement;
//}
//GUIElement* GUIHierarchy::TryRemoveElement(const ECS::EntityID id)
//{
//	if (IsLayerRootID(id))
//	{
//		Assert(this, false, std::format("Tried to remove element with id:'{}' from hierarchy, "
//			"but it is a layer root element and those cannot be removed", EntityData::ToString(id)));
//		return nullptr;
//	}
//
//	//Note: since we need to find the parent in order to know shat child to remove (since we have to be a level up
//	//to know what child to remove since we do NOT want to remove all children) it will be impossible
//	//to remove the layer roots since those do not have any parents
//	size_t childIndex = 0;
//	GUIElement* foundElement = FindParentMutable(id, &childIndex);
//	if (foundElement == nullptr) return nullptr;
//
//	EntityData* childPopped = foundElement->GetEntitySafeMutable().TryPopChildAt(childIndex);
//	if (childPopped == nullptr) return nullptr;
//
//	GUIElement* childAsElement = childPopped->GetEntitySafeMutable().TryGetComponentMutable<GUIElement>();
//	if (childAsElement == nullptr) return nullptr;
//
//	m_OnElementRemoved.Invoke(childAsElement);
//	return childAsElement;
//}
void UIHierarchy::ClearLayer(const UILayer layer)
{
	if (!IsValidLayer(layer, true)) return;
	if (m_layerRoots[layer] == nullptr) return;

	m_layerRoots[layer]->GetEntityMutable().PopAllChildren();
}
//GUIElement* GUIHierarchy::TryGetElementMutablw(const GUILayer layer, const ECS::EntityID id)
//{
//	auto layerIt = m_layerRoots.find(layer);
//	if (layerIt == m_layerRoots.end()) return nullptr;
//
//	return layerIt->second->TryGetElementRecursiveMutable(id);
//}


//RenderInfo GUIHierarchy::CalculateRenderInfo(const GUIElement& element, const RenderInfo& parentInfo) const
//{
//	return RenderInfo(parentInfo.m_TopLeftPos + GetSizeFromFactor(Abs(element.GetRect().GetTopLeftPos().GetPos() -
//		NormalizedPosition::TOP_LEFT), parentInfo.m_RenderSize), element.GetRect().GetSize(parentInfo.m_RenderSize));
//}
//RenderInfo GUIHierarchy::CalculateChildRenderInfo(const GUIElement& element, const RenderInfo& thisRenderInfo) const
//{
//	const RelativeGUIPadding& padding = element.GetPadding();
//	if (!padding.HasNonZeroPadding()) return thisRenderInfo;
//
//	const ScreenPosition paddingTopLeft = ScreenPosition(padding.m_Left.GetValue() * thisRenderInfo.m_RenderSize.m_X,
//		padding.m_Top.GetValue() * thisRenderInfo.m_RenderSize.m_Y);
//	const ScreenPosition paddingBottomRight = ScreenPosition(padding.m_Right.GetValue() * thisRenderInfo.m_RenderSize.m_X,
//		padding.m_Bottom.GetValue() * thisRenderInfo.m_RenderSize.m_Y);
//	//if (padding.HasNonZeroPadding())
//	//{
//	//	//Assert(false, std::format("NOn zero padding tTL:{} bR:{} padding:{}", paddingTopLeft.ToString(), paddingBottomRight.ToString(), m_padding.ToString()));
//	//}
//
//	return RenderInfo(thisRenderInfo.m_TopLeftPos + paddingTopLeft, thisRenderInfo.m_RenderSize - paddingTopLeft - paddingBottomRight);
//}

/*
bool UIHierarchy::TryCalculateElementRectHelper(const ECS::EntityID targetId, const UIRect& parentInfo,
	UITransformData& currentElement, UIRect& outInfo) const
{
	if (currentElement.GetEntity().GetId() == targetId)
	{
		outInfo = m_uiTransformSystem->CalculateRect(currentElement, parentInfo);
		return true;
	}

	if (currentElement.GetEntity().GetChildCount() == 0) return false;

	bool elementWasFound = false;
	for (auto& child : currentElement.GetEntityMutable().GetChildrenOfTypeMutable<UITransformData>())
	{
		if (child == nullptr) continue;

		elementWasFound= TryCalculateElementRectHelper(targetId, m_uiTransformSystem->CalculateChildRect(currentElement, parentInfo), *child, outInfo);
		if (elementWasFound) return true;
	}

	return false;
}
*/

//UIRect UIHierarchy::TryCalculateElementRenderInfoExisting(const UITransformData& element) const
//{
//	UIRect result = {};
//	bool wasFound = false;
//	for (size_t i=0; i<m_layerRoots.size(); i++)
//	{
//		if (m_layerRoots[i] == nullptr) continue;
//		if (m_layerRoots[i]->GetEntity().GetChildCount() == 0) continue;
//
//		wasFound = TryCalculateElementRectHelper(element.GetEntity().GetId(), GetRootRect(), *m_layerRoots[i], result);
//		if (wasFound) return result;
//	}
//
//	return result;
//}
/**/

std::optional<UIRect> TryCalculateRenderRectHelper(const UITransformData& currentTransform, const UIRect& parentRect, const ECS::EntityID targetId)
{
	UIRect thisRect= currentTransform.CalculateRect(parentRect);
	if (currentTransform.GetEntityID() == targetId) return thisRect;

	std::optional<UIRect> childResult = std::nullopt;
	for (const auto& child : currentTransform.GetEntity().GetChildrenOfType<UITransformData>())
	{
		childResult = TryCalculateRenderRectHelper(currentTransform, currentTransform.CalculateChildRect(thisRect), targetId);
		if (childResult != std::nullopt) return childResult;
	}
	return std::nullopt;
}
std::optional<UIRect> UIHierarchy::TryCalculateRenderRect(const UITransformData& element)
{
	std::optional<UIRect> layerResult = std::nullopt;
	for (const auto& layer : m_layerRoots)
	{
		layerResult = TryCalculateRenderRectHelper(*layer, GetRootRect(), element.GetEntityID());
		if (layerResult != std::nullopt) return layerResult;
	}
	return std::nullopt;
}

//void UIHierarchy::UpdateElementHelper(const float deltaTime, UITransformData& element)
//{
//	//If we have invalid rect, we set to max size to ensure rendering works
//	const Vec2 size = element.GetRect().GetSize().GetPos();
//	if (Utils::ApproximateEqualsF(size.m_X, 0) || Utils::ApproximateEqualsF(size.m_Y, 0))
//	{
//		//RelativeUIRect before = m_relativeRect;
//		//Assert(false, std::format("Tried to set panel:{} of size:{} to max", std::to_string(m_id), m_relativeRect.GetSize().ToString()));
//		element.SetMaxSize();
//		//LogWarning(std::format("Changed x for rect of id:{} to:{} -> {}", std::to_string(m_id), before.ToString(), m_relativeRect.ToString()));
//	}
//
//	//element.Update(deltaTime);
//
//	if (element.GetEntity().GetChildCount()==0) return;
//	for (auto& child : element.GetEntityMutable().GetChildrenOfTypeMutable<UITransformData>())
//	{
//		if (child == nullptr) continue;
//		UpdateElementHelper(deltaTime, *child);
//	}
//}

/*
void UIHierarchy::UpdateAll(const float deltaTime)
{
	try
	{
		LogWarning(std::format("Before update:{}", ToStringTree()));

		//Since we are in increasing order and lower number -> farther in background
		for(size_t i = 0; i < m_layerRoots.size(); i++)
		{
			//LogWarning(std::format("Attempting [{}/{}] to updarte layer root:{}. hierarcxhy:{}", std::to_string(i), std::to_string(m_layerRoots.size()),
			//m_layerRoots[i]!=nullptr? m_layerRoots[i]->ToStringBase() : "NULL", ToStringTree()));
				//if (m_layerRoots[i] == nullptr) continue;
				//{
				//	Assert(false, std::format("Tried to update all gui elements in hierarchy, "
				//		"but layer root was null"));
				//	return;
				//}
			if (m_layerRoots[i] == nullptr) continue;
			if (m_layerRoots[i]->GetEntity().GetChildCount() == 0) continue;

			UpdateElementHelper(deltaTime, *m_layerRoots[i]);

			LogWarning(std::format("Once finished update tree:{}", ToStringTree()));
		}
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to run update all in gui hierarchy "
			"but ran into error:{} full tree:{}", e.what(), ToStringTree()));
	}

	LogError(std::format("Tree:{}", ToStringTree()));
	//Assert(false, std::format("After first update tree:{}", ToStringTree()));
}
*/

UIRect UIHierarchy::GetRootRect() const
{
	return UIRect(ScreenPosition(0, 0), m_rootSize);
}

std::string UIHierarchy::ToStringElementHelper(std::string startNewLine, const UITransformData& element) const
{
	std::string result = "";
	result += std::format("\n{}-> {}", startNewLine, element.ToString());

	if (element.GetEntity().GetChildCount()==0) return result;

	startNewLine += "    ";
	for (const auto& child : element.GetEntity().GetChildrenOfType<UITransformData>())
	{
		if (child == nullptr) continue;
		result += ToStringElementHelper(startNewLine, *child);
	}

	return result;
}

std::string UIHierarchy::ToStringTree() const
{
	std::string result = std::format("\n[Layers:{}]", std::to_string(m_layerRoots.size()));

	for (size_t i = 0; i < m_layerRoots.size(); i++)
	{
		result += std::format("\n\nLayer{}{}:{}", std::to_string(i),
			i== BOTTOM_LAYER? "(BOTTOM)" : "",
			m_layerRoots[i]!=nullptr? ToStringElementHelper("", *m_layerRoots[i]) : "NULL");
	}
	return result;
}