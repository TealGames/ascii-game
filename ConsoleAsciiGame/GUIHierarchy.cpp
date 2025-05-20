#include "pch.hpp"
#include "GUIHierarchy.hpp"
#include "Debug.hpp"
#include <queue>
#include <vector>

GUIHierarchy::GUIHierarchy(const Vec2Int rootCanvasSize) 
	: m_rootSize(rootCanvasSize), m_layerRoots({}), m_OnElementAdded(), m_OnElementRemoved()
{
	m_rootElements.reserve(TOP_LAYER);
}


GUIElementLayersCollection::iterator GUIHierarchy::CreateNewLayer(const GUILayer layer)
{
	m_rootElements.emplace_back(ContainerGUI());
	m_rootElements.back().m_OnFarthestChildElementAttached.AddListener([this](GUIElement* el)-> void
		{
			if (el == nullptr)
			{
				Assert(false, std::format("Invoked new gui element added with null element"));
				return;
			}

			m_OnElementAdded.Invoke(el);
		});

	return m_layerRoots.emplace(layer, &m_rootElements.back()).first;
}
bool GUIHierarchy::IsLayerRootID(const GUIElementID id) const
{
	//TODO: we could speed up this search if we reserve each layer with its own range
	
	//NOTE: we could go through the root of each layer OR we can just look at cached
	//root elements, but we do root elements since they are faster to access
	if (m_rootElements.empty()) return false;

	for (const auto& rootElement : m_rootElements)
	{
		if (rootElement.GetId() == id)
			return true;
	}
	return false;
}

GUIElement* GUIHierarchy::FindMutable(const GUIElementID id)
{
	//TODO: there could be a speed up by having each layer be reserved some amounts of ids so we dont have to search all layers
	GUIElement* foundElement = nullptr;
	for (auto& layer : m_layerRoots)
	{
		foundElement = layer.second->FindRecursiveMutable(id);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}
GUIElement* GUIHierarchy::FindParentMutable(const GUIElementID id, size_t* foundChildIndex)
{
	GUIElement* foundElement = nullptr;
	for (auto& layer : m_layerRoots)
	{
		foundElement = layer.second->FindParentRecursiveMutable(id, foundChildIndex);
		if (foundElement != nullptr) return foundElement;
	}
	return nullptr;
}

std::vector<GUIElement*> GUIHierarchy::GetLayerRootsMutable(const bool topLayerFirst)
{
	if (m_layerRoots.empty()) return {};
	std::vector<GUIElement*> layers = {};


	if (topLayerFirst)
	{
		auto it = --m_layerRoots.end();
		while (true)
		{
			layers.push_back(it->second);
			if (it == m_layerRoots.begin()) return layers;

			it--;
		}
	}
	else
	{
		for (auto& layer : m_layerRoots)
			layers.push_back(layer.second);
	}
	return layers;
}

//void GUIHierarchy::SetRoot(const GUILayer layer, GUIElement* root)
//{
//	if (root == nullptr) return;
//	auto rootIt = m_layerRoots.find(layer);
//
//	if (rootIt == m_layerRoots.end()) rootIt = CreateNewLayer(layer);
//	rootIt->second = root;
//}
void GUIHierarchy::AddToRoot(const GUILayer layer, GUIElement* element)
{
	//Note: we could use the existing functions to accomplish this, but we can make assumptions
	//can speed up some of the process here
	if (element == nullptr)
	{
		Assert(false, std::format("Tried to add element to root of gui hierarchy but element is NULL"));
		return;
	}

	auto layerIt = m_layerRoots.find(layer);
	if (layerIt == m_layerRoots.end())
		layerIt = CreateNewLayer(layer);

	if (layerIt->second == nullptr)
	{
		Assert(false, std::format("Tried to add element to root of gui hierarchy but root is null"));
		return;
	}

	layerIt->second->PushChild(element);
	//m_OnElementAdded.Invoke(element);
}

bool GUIHierarchy::TryAddElementAsChild(const GUIElementID parentId, GUIElement* element)
{
	if (element == nullptr)
	{
		Assert(false, std::format("Tried to add element as child to root of gui hierarchy but element is NULL"));
		return false;
	}

	GUIElement* foundElement = FindMutable(parentId);
	if (foundElement == nullptr) return false;

	foundElement->PushChild(element);
	//m_OnElementAdded.Invoke(element);
	return true;
}
GUIElement* GUIHierarchy::TryRemoveElementChildren(const GUIElementID id)
{
	GUIElement* foundElement = FindMutable(id);
	if (foundElement == nullptr) return nullptr;

	for (const auto& child : foundElement->PopAllChildren())
		m_OnElementRemoved.Invoke(child);

	return foundElement;
}
GUIElement* GUIHierarchy::TryRemoveElement(const GUIElementID id)
{
	if (IsLayerRootID(id))
	{
		Assert(this, false, std::format("Tried to remove element with id:'{}' from hierarchy, "
			"but it is a layer root element and those cannot be removed", std::to_string(id)));
		return nullptr;
	}

	//Note: since we need to find the parent in order to know shat child to remove (since we have to be a level up
	//to know what child to remove since we do NOT want to remove all children) it will be impossible
	//to remove the layer roots since those do not have any parents
	size_t childIndex = 0;
	GUIElement* foundElement = FindParentMutable(id, &childIndex);
	if (foundElement == nullptr) return nullptr;

	GUIElement* childPopped= foundElement->TryPopChildAt(childIndex);
	m_OnElementRemoved.Invoke(childPopped);
	return childPopped;
}
void GUIHierarchy::ClearLayer(const GUILayer layer)
{
	auto layerIt = m_layerRoots.find(layer);
	if (layerIt == m_layerRoots.end()) return;

	layerIt->second->PopAllChildren();
}
//GUIElement* GUIHierarchy::TryGetElementMutablw(const GUILayer layer, const GUIElementID id)
//{
//	auto layerIt = m_layerRoots.find(layer);
//	if (layerIt == m_layerRoots.end()) return nullptr;
//
//	return layerIt->second->TryGetElementRecursiveMutable(id);
//}

void GUIHierarchy::ExecuteOnAllElementsDescending(const std::function<void(GUILayer, GUIElement*)>& action)
{
	if (!action) return;

	std::queue<GUIElement*> bfsQueue = {};
	GUIElement* currElement= nullptr;
	std::vector<GUIElement*> layerElements = {};

	auto it = --m_layerRoots.end();
	//Top layer first means we go backwards
	while (true)
	{
		bfsQueue = {};
		layerElements = {};
		bfsQueue.push(it->second);
		while (!bfsQueue.empty())
		{
			currElement = bfsQueue.front();
			//LogWarning(std::format("Going throguh elkements descending el:{}", currElement->ToStringBase()));
			//NOTE: we do NOT want to execute actions on the root elements because then we can 
			//have acess to root and make undesired changes that should not be allowed
			if (!IsLayerRootID(currElement->GetId())) 
				layerElements.push_back(currElement);

			bfsQueue.pop();

			for (auto& child : currElement->GetChildrenMutable())
				bfsQueue.push(child);
		}

		if (!layerElements.empty())
		{
			//Note: since bfs goes top to bottom, this behavior by default will be bigger containers and lower level
			//elements so we must iterate through it backwards
			for (int i = layerElements.size() - 1; i >= 0; i--)
			{
				if (layerElements[i] == nullptr) continue;
				action(it->first, layerElements[i]);
			}
		}
		
		if (it == m_layerRoots.begin()) return;
		it--;
	}
}

bool GUIHierarchy::TryCalculateElementRenderInfoHelper(const GUIElementID targetId, const RenderInfo& parentInfo,
	GUIElement& currentElement, RenderInfo& outInfo) const
{
	if (currentElement.GetId() == targetId)
	{
		outInfo = currentElement.CalculateRenderInfo(parentInfo);
		return true;
	}

	if (currentElement.GetChildCount() == 0) return false;

	bool elementWasFound = false;
	for (auto& child : currentElement.GetChildrenMutable())
	{
		if (child == nullptr) continue;

		elementWasFound= TryCalculateElementRenderInfoHelper(targetId, currentElement.CalculateRenderInfo(parentInfo), *child, outInfo);
		if (elementWasFound) return true;
	}

	return false;
}

RenderInfo GUIHierarchy::TryCalculateElementRenderInfo(const GUIElement& element) const
{
	RenderInfo result = {};
	bool wasFound = false;
	for (const auto& layerRoot : m_layerRoots)
	{
		if (layerRoot.second == nullptr) continue;
		if (layerRoot.second->GetChildCount() == 0) continue;

		wasFound = TryCalculateElementRenderInfoHelper(element.GetId(), GetRootRenderInfo(), *layerRoot.second, result);
		if (wasFound) return result;
	}

	return result;
}

void GUIHierarchy::UpdateAll(const float deltaTime)
{
	try
	{
		//Since we are in increasing order and lower number -> farther in background
		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i]->GetChildCount() == 0) continue;

			m_layerRoots[i]->UpdateRecursive(deltaTime);
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

RenderInfo GUIHierarchy::GetRootRenderInfo() const
{
	return RenderInfo(ScreenPosition(0, 0), m_rootSize);
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
			if (m_layerRoots[i]->GetChildCount() == 0) continue;

			//Note: the top left pos is (0, 0) when rendering and y increases as you go down
			m_layerRoots[i]->RenderRecursive(GetRootRenderInfo());
		}
		//Assert(false, std::format("Reender all rec"));
	}
	catch(const std::exception& e)
	{
		Assert(false, std::format("Tried to run render all in gui hierarchy "
			"but ran into error:{} full tree:{}", e.what(), ToStringTree()));
	}
}

std::string GUIHierarchy::ToStringTree() const
{
	std::string result = "";
	for (const auto& layer : m_layerRoots)
	{
		result += std::format("\n\nLayer:{}", layer.second->ToStringRecursive(""));
	}
	return result;
}