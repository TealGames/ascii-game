#include "pch.hpp"
#include "GUIHierarchy.hpp"
#include "Debug.hpp"
#include <queue>
#include <vector>
#include "GlobalEntityManager.hpp"
//#include "EntityData.hpp"

static constexpr bool DRAW_RENDER_BOUNDS = false;

GUIHierarchy::GUIHierarchy(const ECS::UITransformSystem& uiTransformSystem, ECS::UIRenderSystem& uiRenderSystem,
	GlobalEntityManager& globalEntityManager, const Vec2Int rootCanvasSize)
	: m_uiTransformSystem(&uiTransformSystem), m_uiRenderSystem(&uiRenderSystem), 
	m_uiRoot(nullptr), m_rootSize(rootCanvasSize), m_layerRoots({}), m_OnElementAdded(), m_OnElementRemoved()
{
	//m_rootElements.reserve(TOP_LAYER);
	EntityData& uiRootEntity= globalEntityManager.CreateGlobalEntity(ROOT_UI_ENTITY_NAME, TransformData());
	m_uiRoot = &(uiRootEntity.AddComponent<UITransformData>());
	//m_rootElements.reserve(MAX_LAYERS);
}

bool GUIHierarchy::IsValidLayer(const GUILayer layer, const bool logError) const
{
	const bool isValid= layer <= TOP_LAYER;
	if (logError)
	{
		Assert(this, false, std::format("Attempted to use a gui layer:{} "
			"but the underlying layer value is invalid", std::to_string(layer)));
	}
	return isValid;
}

UITransformData* GUIHierarchy::CreateNewLayer(const GUILayer layer)
{
	if (!IsValidLayer(layer, true)) return nullptr;
	if (m_layerRoots[layer]!=nullptr)
	{
		Assert(false, std::format("Attempted to create a new layer:{} in gui hierarchy "
			"but that layer has already been created", std::to_string(layer)));
		return nullptr;
	}

	EntityData& layerRoot= m_uiRoot->GetEntitySafeMutable().CreateChild(std::format("Layer{}", std::to_string(layer)), TransformData());
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
bool GUIHierarchy::IsLayerRootID(const ECS::EntityID id) const
{
	//TODO: we could speed up this search if we reserve each layer with its own range
	
	//NOTE: we could go through the root of each layer OR we can just look at cached
	//root elements, but we do root elements since they are faster to access
	if (m_uiRoot->GetEntitySafe().GetChildCount()==0) return false;

	for (const auto& rootElement : m_layerRoots)
	{
		if (rootElement->GetEntitySafe().GetId() == id)
			return true;
	}
	return false;
}

UITransformData* GUIHierarchy::FindMutable(const ECS::EntityID id)
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

		foundElement = m_layerRoots[i]->GetEntitySafeMutable().FindComponentRecursiveMutable<UITransformData>(id);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}
UITransformData* GUIHierarchy::FindParentMutable(const ECS::EntityID id, size_t* foundChildIndex)
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

		foundEntity = m_layerRoots[i]->GetEntitySafeMutable().FindParentComponentRecursiveMutable<UITransformData>(id, foundChildIndex);
		if (foundEntity != nullptr) return foundEntity;
	}
	return nullptr;
}

std::vector<UITransformData*> GUIHierarchy::GetLayerRootsMutable(const bool topLayerFirst)
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

const Vec2Int GUIHierarchy::GetRootSize() const { return m_rootSize; }
const UITransformData* GUIHierarchy::GetRootElement() const { return m_uiRoot; }
const UITransformData* GUIHierarchy::GetLayerRootElement(const GUILayer layer) const
{
	if (!IsValidLayer(layer, true)) return nullptr;
	return m_layerRoots[layer];
}

//void GUIHierarchy::SetRoot(const GUILayer layer, GUIElement* root)
//{
//	if (root == nullptr) return;
//	auto rootIt = m_layerRoots.find(layer);
//
//	if (rootIt == m_layerRoots.end()) rootIt = CreateNewLayer(layer);
//	rootIt->second = root;
//}
void GUIHierarchy::AddToRoot(const GUILayer layer, UITransformData* element)
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

	m_layerRoots[layer]->GetEntitySafeMutable().PushChild(element->GetEntitySafeMutable());
	//m_OnElementAdded.Invoke(element);
}
UITransformData* GUIHierarchy::RemoveFromRoot(const GUILayer layer, const ECS::EntityID id)
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

	return m_layerRoots[layer]->GetEntitySafeMutable().TryPopChildAs<UITransformData>(id);
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
void GUIHierarchy::ClearLayer(const GUILayer layer)
{
	if (!IsValidLayer(layer, true)) return;
	if (m_layerRoots[layer] == nullptr) return;

	m_layerRoots[layer]->GetEntitySafeMutable().PopAllChildren();
}
//GUIElement* GUIHierarchy::TryGetElementMutablw(const GUILayer layer, const ECS::EntityID id)
//{
//	auto layerIt = m_layerRoots.find(layer);
//	if (layerIt == m_layerRoots.end()) return nullptr;
//
//	return layerIt->second->TryGetElementRecursiveMutable(id);
//}

void GUIHierarchy::ExecuteOnAllElementsDescending(const std::function<void(GUILayer, UITransformData&)>& action)
{
	if (!action) return;

	std::queue<UITransformData*> bfsQueue = {};
	UITransformData* currElement= nullptr;
	std::vector<UITransformData*> layerElements = {};

	int i = m_layerRoots.size() - 1;
	//Top layer first means we go backwards
	while (i>=0)
	{
		bfsQueue = {};
		layerElements = {};

		while (i>=0 && m_layerRoots[i] == nullptr) i--;
		if (i < 0) return;

		bfsQueue.push(m_layerRoots[i]);
		while (!bfsQueue.empty())
		{
			currElement = bfsQueue.front();
			//LogWarning(std::format("Going throguh elkements descending el:{}", currElement->ToStringBase()));
			//NOTE: we do NOT want to execute actions on the root elements because then we can 
			//have acess to root and make undesired changes that should not be allowed
			if (!IsLayerRootID(currElement->GetEntitySafe().GetId())) 
				layerElements.push_back(currElement);

			bfsQueue.pop();

			for (auto& child : currElement->GetEntitySafeMutable().GetChildrenOfTypeMutable<UITransformData>())
			{
				if (child == nullptr) continue;
				bfsQueue.push(child);
			}
		}

		if (!layerElements.empty())
		{
			//Note: since bfs goes top to bottom, this behavior by default will be bigger containers and lower level
			//elements so we must iterate through it backwards
			for (int i = layerElements.size() - 1; i >= 0; i--)
			{
				if (layerElements[i] == nullptr) continue;
				action(static_cast<GUILayer>(i), *layerElements[i]);
			}
		}
		
		i--;
	}
}

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

bool GUIHierarchy::TryCalculateElementRectHelper(const ECS::EntityID targetId, const GUIRect& parentInfo,
	UITransformData& currentElement, GUIRect& outInfo) const
{
	if (currentElement.GetEntitySafe().GetId() == targetId)
	{
		outInfo = m_uiTransformSystem->CalculateRect(currentElement, parentInfo);
		return true;
	}

	if (currentElement.GetEntitySafe().GetChildCount() == 0) return false;

	bool elementWasFound = false;
	for (auto& child : currentElement.GetEntitySafeMutable().GetChildrenOfTypeMutable<UITransformData>())
	{
		if (child == nullptr) continue;

		elementWasFound= TryCalculateElementRectHelper(targetId, m_uiTransformSystem->CalculateChildRect(currentElement, parentInfo), *child, outInfo);
		if (elementWasFound) return true;
	}

	return false;
}

GUIRect GUIHierarchy::TryCalculateElementRenderInfoExisting(const UITransformData& element) const
{
	GUIRect result = {};
	bool wasFound = false;
	for (size_t i=0; i<m_layerRoots.size(); i++)
	{
		if (m_layerRoots[i] == nullptr) continue;
		if (m_layerRoots[i]->GetEntitySafe().GetChildCount() == 0) continue;

		wasFound = TryCalculateElementRectHelper(element.GetEntitySafe().GetId(), GetRootRect(), *m_layerRoots[i], result);
		if (wasFound) return result;
	}

	return result;
}
GUIRect GUIHierarchy::TryCalculateElementRenderInfoFromRoot(const UITransformData& element)
{
	return m_uiTransformSystem->CalculateRect(element, GetRootRect());
}

void GUIHierarchy::UpdateElementHelper(const float deltaTime, UITransformData& element)
{
	//If we have invalid rect, we set to max size to ensure rendering works
	const Vec2 size = element.GetRect().GetSize().GetPos();
	if (Utils::ApproximateEqualsF(size.m_X, 0) || Utils::ApproximateEqualsF(size.m_Y, 0))
	{
		//RelativeGUIRect before = m_relativeRect;
		//Assert(false, std::format("Tried to set panel:{} of size:{} to max", std::to_string(m_id), m_relativeRect.GetSize().ToString()));
		element.SetMaxSize();
		//LogWarning(std::format("Changed x for rect of id:{} to:{} -> {}", std::to_string(m_id), before.ToString(), m_relativeRect.ToString()));
	}

	//element.Update(deltaTime);

	if (element.GetEntitySafe().GetChildCount()==0) return;
	for (auto& child : element.GetEntitySafeMutable().GetChildrenOfTypeMutable<UITransformData>())
	{
		if (child == nullptr) continue;
		UpdateElementHelper(deltaTime, *child);
	}
}
void GUIHierarchy::UpdateAll(const float deltaTime)
{
	try
	{
		LogWarning(std::format("Before update:{}", ToStringTree()));

		//Since we are in increasing order and lower number -> farther in background
		for(size_t i = 0; i < m_layerRoots.size(); i++)
		{
			/*LogWarning(std::format("Attempting [{}/{}] to updarte layer root:{}. hierarcxhy:{}", std::to_string(i), std::to_string(m_layerRoots.size()),
				m_layerRoots[i]!=nullptr? m_layerRoots[i]->ToStringBase() : "NULL", ToStringTree()));*/
				//if (m_layerRoots[i] == nullptr) continue;
				//{
				//	Assert(false, std::format("Tried to update all gui elements in hierarchy, "
				//		"but layer root was null"));
				//	return;
				//}
			if (m_layerRoots[i] == nullptr) continue;
			if (m_layerRoots[i]->GetEntitySafe().GetChildCount() == 0) continue;

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

GUIRect GUIHierarchy::GetRootRect() const
{
	return GUIRect(ScreenPosition(0, 0), m_rootSize);
}

void GUIHierarchy::RenderElementHelper(UIRendererData& renderer, const GUIRect& parentInfo)
{
	UITransformData& transform = *(renderer.GetEntitySafeMutable().TryGetComponentMutable<UITransformData>());
	//We get this render info based on this relative pos
	const GUIRect thisRenderRect = m_uiTransformSystem->CalculateRect(transform, parentInfo);

	/*if (DRAW_RENDER_BOUNDS) DrawRectangleLines(thisRenderInfo.m_TopLeftPos.m_X, thisRenderInfo.m_TopLeftPos.m_Y,
		thisRenderInfo.GetSize().m_X, thisRenderInfo.GetSize().m_Y, YELLOW);*/

	const GUIRect actualRenderedArea= m_uiRenderSystem->RenderUIEntity(renderer, parentInfo);
	renderer.SetLastRenderRect(actualRenderedArea);
	/*LogError(std::format("Rendering element calculated from parent info:{} rect:{} current info:{}",
		renderInfo.ToString(), m_relativeRect.ToString(), thisRenderInfo.ToString()));*/

	if (transform.GetEntitySafe().GetChildCount()==0) return;

	const GUIRect childRenderRect = m_uiTransformSystem->CalculateChildRect(transform, thisRenderRect);
	/*if (m_padding.HasNonZeroPadding())
	{
		Assert(false, std::format("Padding:{} this render:{} child render:{}",
			m_padding.ToString(), thisRenderInfo.ToString(), childRenderInfo.ToString()));
	}*/

	//TODO: this is really slow to have to check every element in the ui tree if it has the renderer. we 
	//should instead cache a different renderer tree for optimization
	for (auto& child : transform.GetEntitySafeMutable().GetChildrenOfTypeMutable<UIRendererData>())
	{
		if (child == nullptr) continue;
		//Note: we assume space reserved for this element is the space it actually 
		//used for render (and thus can be used for children) because we do not want size
		//changes during render especially becuase of normalized vs in-game space coordinates
		RenderElementHelper(*child, childRenderRect);
	}
}
void GUIHierarchy::RenderAll()
{
	//TODO: this could probably be optimized if there are no changes allowed during runtime
	//by having a priority queue or map of all values to render in order

	try
	{
		//Since we are in increasing order and lower number -> farther in background
		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i] == nullptr) continue;
			if (m_layerRoots[i]->GetEntitySafe().GetChildCount() == 0) continue;

			//Note: the top left pos is (0, 0) when rendering and y increases as you go down
			if (UIRendererData* renderer= m_layerRoots[i]->GetEntitySafeMutable().TryGetComponentMutable<UIRendererData>())
				RenderElementHelper(*renderer, GetRootRect());
		}
		//Assert(false, std::format("Reender all rec"));
	}
	catch(const std::exception& e)
	{
		Assert(false, std::format("Tried to run render all in gui hierarchy "
			"but ran into error:{} full tree:{}", e.what(), ToStringTree()));
	}
}

std::string GUIHierarchy::ToStringElementHelper(std::string startNewLine, const UITransformData& element) const
{
	std::string result = "";
	result += std::format("\n{}-> {}", startNewLine, element.ToString());

	if (element.GetEntitySafe().GetChildCount()==0) return result;

	startNewLine += "    ";
	for (const auto& child : element.GetEntitySafe().GetChildrenOfType<UITransformData>())
	{
		if (child == nullptr) continue;
		result += ToStringElementHelper(startNewLine, *child);
	}

	return result;
}

std::string GUIHierarchy::ToStringTree() const
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