#pragma once
#include "ECS/Component/Types/UI/UIContainer.hpp"
#include "ECS/Systems/Types/UI/UITransformSystem.hpp"
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"
#include <cstdint>
#include <map>
#include <limits>
#include <functional>
#include <optional>
#include <queue>
#include "Utils/Data/Event.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

using UILayer = std::uint8_t;
constexpr UILayer MAX_LAYERS = 8;

constexpr UILayer TOP_LAYER = MAX_LAYERS - 1;
constexpr UILayer BOTTOM_LAYER = std::numeric_limits<UILayer>::min();
constexpr UILayer DEFAULT_LAYER = BOTTOM_LAYER;

class GlobalEntityManager;
//TODO: since layer is an int, instead use vector and indexing
using UIElementLayersCollection = std::array<UITransformData*, MAX_LAYERS>;
class UIHierarchy
{
private:
	GlobalEntityManager* m_globalEntityManager;
	Vec2Int m_rootSize;
	UITransformData* m_uiRoot;
	/// <summary>
	/// Stores each of the layer's gui element roots. Note: layers are stored in increasing priority order 
	/// (meaning background layers first, then foreground layers)
	/// </summary>
	UIElementLayersCollection m_layerRoots;
	//std::vector<GUIElement*> m_rootElements;
public:
	static constexpr const char* ROOT_UI_ENTITY_NAME = "RootUI";
	/// <summary>
	/// Invokes when an element is added to THE TREE.
	/// This means that any adds to gui elements by themselves will not trigger it
	/// unless it is done by this class
	/// </summary>
	Event<void, const UITransformData*> m_OnElementAdded;
	/// <summary>
	/// Invokes when an element is removed from THE TREE.
	/// This means that any removes from gui elements by themselves will not trigger it
	/// unless it is done by this class
	/// </summary>
	Event<void, const UITransformData*> m_OnElementRemoved;

private:
	bool IsValidLayer(const UILayer layer, const bool logError) const;

	UITransformData* CreateNewLayer(const UILayer layer);
	void ClearLayer(const UILayer layer);
	bool IsLayerRootID(const ECS::EntityID id) const;

	/*bool TryCalculateElementRectHelper(const ECS::EntityID targetId, const UIRect& parentInfo,
		UITransformData& currentElement, UIRect& outInfo) const;*/
	/*RenderInfo CalculateRenderInfo(const GUIElement& element, const RenderInfo& parentInfo) const;
	RenderInfo CalculateChildRenderInfo(const GUIElement& element, const RenderInfo& thisRenderInfo) const;*/

	/*void UpdateElementHelper(const float deltaTime, UITransformData& element);
	void RenderElementHelper(UIRendererData& renderer, const UIRect& parentInfo);*/
	std::string ToStringElementHelper(std::string startNewLine, const UITransformData& element) const;
public:
	UIHierarchy(GlobalEntityManager& globalEntityManager, const Vec2Int rootCanvasSize);

	void Init();

	const Vec2Int GetRootSize() const;
	const UITransformData* GetRootElement() const;
	UIRect GetRootRect() const;
	const UITransformData* GetLayerRootElement(const UILayer layer) const;
	std::tuple<EntityData*, UITransformData*> CreateAtRoot(const UILayer layer, const std::string& name);
	void AddToRoot(const UILayer layer, UITransformData* element);
	UITransformData* RemoveFromRoot(const UILayer layer, const ECS::EntityID id);

	UITransformData* FindMutable(const ECS::EntityID id);
	UITransformData* FindParentMutable(const ECS::EntityID id, size_t* foundChildIndex =nullptr);
	std::vector<UITransformData*> GetLayerRootsMutable(const bool topLayerFirst);
	/// <summary>
	/// Will add the element as a child of the gui element with the id
	/// Note: will return false if the parent id is not found and true otherwise
	/// </summary>
	/// <param name="layer"></param>
	/// <param name="parentId"></param>
	/// <param name="element"></param>
	//bool TryAddElementAsChild(const ECS::EntityID parentId, GUIElement& element);
	/// <summary>
	/// Will remove the element's children from the tree
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	//GUIElement* TryRemoveElementChildren(const ECS::EntityID id);
	/// <summary>
	/// Will remove this element from the tree including all of its children
	/// Note: removing the root element of a layer is NOT allowed
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	//GUIElement* TryRemoveElement(const ECS::EntityID id);

	/// <summary>
	/// Will execute the action on all elements (except each layer's root) 
	/// based on their order in the layer and going from top layer to bottom layer
	/// </summary>
	template<typename T>
	requires std::is_base_of_v<Component, T>
	void ElementTraversalBFS(const std::function<void(UILayer, T&)>& action)
	{
		if (!action) return;

		std::queue<T*> bfsQueue = {};
		T* currElement = nullptr;
		std::vector<T*> layerElements = {};

		int i = m_layerRoots.size() - 1;
		//Top layer first means we go backwards
		while (i >= 0)
		{
			bfsQueue = {};
			layerElements = {};

			while (i >= 0 && m_layerRoots[i] == nullptr) i--;
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

				for (auto& child : currElement->GetEntitySafeMutable().GetChildrenOfTypeMutable<T>())
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
					//if (layerElements[i] == nullptr) continue;
					action(static_cast<UILayer>(i), *layerElements[i]);
				}
			}

			i--;
		}
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	void ElementTraversalDFS(const std::function<void(UILayer, T&)>& action)
	{
		std::function<void(UILayer, EntityData&)> HelperDFS = [&](UILayer layer, EntityData& entity)-> void
			{
				if (T* maybeT = entity.TryGetComponentMutable<T>()) action(layer, *maybeT);

				if (entity.GetChildCount() == 0) return;
				for (auto& child : entity.GetChildrenMutable())
				{
					HelperDFS(layer, *child);
				}
			};

		for (size_t i=0; i<m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i] == nullptr) continue;
			HelperDFS(static_cast<UILayer>(i), m_layerRoots[i]->GetEntityMutable());
		}
	}

	void LayerTraversal(const std::function<void(UILayer, UITransformData&)>& action);

	/// <summary>
	/// Will calculate the actual size of the element AS LONG AS IT EXISTS WITHIN THE TREE
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	/*UIRect TryCalculateElementRenderInfoExisting(const UITransformData& element) const;
	/// <summary>
	/// Will calculate the element as if it is a root element
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	
	void UpdateAll(const float deltaTime);
	*/
	std::optional<UIRect> TryCalculateRenderRect(const UITransformData& element);


	//void RenderAll();
	std::string ToStringTree() const;
};

