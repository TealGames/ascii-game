#pragma once
#include "UIContainer.hpp"
#include "UITransformSystem.hpp"
#include "UIRendererSystem.hpp"
#include <cstdint>
#include <map>
#include <limits>
#include <functional>
#include "Event.hpp"
#include "EntityData.hpp"

using GUILayer = std::uint8_t;
constexpr GUILayer MAX_LAYERS = 8;

constexpr GUILayer TOP_LAYER = MAX_LAYERS - 1;
constexpr GUILayer BOTTOM_LAYER = std::numeric_limits<GUILayer>::min();
constexpr GUILayer DEFAULT_LAYER = BOTTOM_LAYER;

class GlobalEntityManager;
//TODO: since layer is an int, instead use vector and indexing
using GUIElementLayersCollection = std::array<UITransformData*, MAX_LAYERS>;
class GUIHierarchy
{
private:
	const ECS::UITransformSystem* m_uiTransformSystem;
	ECS::UIRenderSystem* m_uiRenderSystem;

	Vec2Int m_rootSize;
	UITransformData* m_uiRoot;
	/// <summary>
	/// Stores each of the layer's gui element roots. Note: layers are stored in increasing priority order 
	/// (meaning background layers first, then foreground layers)
	/// </summary>
	GUIElementLayersCollection m_layerRoots;
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
	bool IsValidLayer(const GUILayer layer, const bool logError) const;

	UITransformData* CreateNewLayer(const GUILayer layer);
	void ClearLayer(const GUILayer layer);
	bool IsLayerRootID(const ECS::EntityID id) const;
	GUIRect GetRootRect() const;

	bool TryCalculateElementRectHelper(const ECS::EntityID targetId, const GUIRect& parentInfo,
		UITransformData& currentElement, GUIRect& outInfo) const;
	/*RenderInfo CalculateRenderInfo(const GUIElement& element, const RenderInfo& parentInfo) const;
	RenderInfo CalculateChildRenderInfo(const GUIElement& element, const RenderInfo& thisRenderInfo) const;*/

	void UpdateElementHelper(const float deltaTime, UITransformData& element);
	void RenderElementHelper(UIRendererData& renderer, const GUIRect& parentInfo);
	std::string ToStringElementHelper(std::string startNewLine, const UITransformData& element) const;
public:
	GUIHierarchy(const ECS::UITransformSystem& uiTransformSystem, ECS::UIRenderSystem& uiRenderSystem, 
		GlobalEntityManager& globalEntityManager, const Vec2Int rootCanvasSize);

	const Vec2Int GetRootSize() const;
	const UITransformData* GetRootElement() const;
	const UITransformData* GetLayerRootElement(const GUILayer layer) const;
	void AddToRoot(const GUILayer layer, UITransformData* element);
	UITransformData* RemoveFromRoot(const GUILayer layer, const ECS::EntityID id);

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
	void ExecuteOnAllElementsDescending(const std::function<void(GUILayer, UITransformData&)>& action);

	/// <summary>
	/// Will calculate the actual size of the element AS LONG AS IT EXISTS WITHIN THE TREE
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	GUIRect TryCalculateElementRenderInfoExisting(const UITransformData& element) const;
	/// <summary>
	/// Will calculate the element as if it is a root element
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	GUIRect TryCalculateElementRenderInfoFromRoot(const UITransformData& element);

	void UpdateAll(const float deltaTime);

	void RenderAll();
	std::string ToStringTree() const;
};

