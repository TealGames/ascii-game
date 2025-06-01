#pragma once
#include "ContainerGUI.hpp"
#include "GUIElement.hpp"
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
using GUIElementLayersCollection = std::array<GUIElement*, MAX_LAYERS>;
class GUIHierarchy
{
private:
	Vec2Int m_rootSize;
	GUIElement* m_uiRoot;
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
	Event<void, const GUIElement*> m_OnElementAdded;
	/// <summary>
	/// Invokes when an element is removed from THE TREE.
	/// This means that any removes from gui elements by themselves will not trigger it
	/// unless it is done by this class
	/// </summary>
	Event<void, const GUIElement*> m_OnElementRemoved;

private:
	bool IsValidLayer(const GUILayer layer, const bool logError) const;

	GUIElement* CreateNewLayer(const GUILayer layer);
	void ClearLayer(const GUILayer layer);
	bool IsLayerRootID(const ECS::EntityID id) const;
	RenderInfo GetRootRenderInfo() const;

	bool TryCalculateElementRenderInfoHelper(const ECS::EntityID targetId, const RenderInfo& parentInfo,
		GUIElement& currentElement, RenderInfo& outInfo) const;
	RenderInfo CalculateRenderInfo(const GUIElement& element, const RenderInfo& parentInfo) const;
	RenderInfo CalculateChildRenderInfo(const GUIElement& element, const RenderInfo& thisRenderInfo) const;

	void UpdateElementHelper(const float deltaTime, GUIElement& element);
	void RenderElementHelper(const RenderInfo& parentInfo, GUIElement& element);
	std::string ToStringElementHelper(std::string startNewLine, const GUIElement& element) const;
public:
	GUIHierarchy(GlobalEntityManager& globalEntityManager, const Vec2Int rootCanvasSize);

	const Vec2Int GetRootSize() const;
	const GUIElement* GetRootElement() const;
	const GUIElement* GetLayerRootElement(const GUILayer layer) const;
	void AddToRoot(const GUILayer layer, GUIElement* element);
	GUIElement* RemoveFromRoot(const GUILayer layer, const ECS::EntityID id);

	GUIElement* FindMutable(const ECS::EntityID id);
	GUIElement* FindParentMutable(const ECS::EntityID id, size_t* foundChildIndex =nullptr);
	std::vector<GUIElement*> GetLayerRootsMutable(const bool topLayerFirst);
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
	void ExecuteOnAllElementsDescending(const std::function<void(GUILayer, GUIElement&)>& action);

	/// <summary>
	/// Will calculate the actual size of the element AS LONG AS IT EXISTS WITHIN THE TREE
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	RenderInfo TryCalculateElementRenderInfoExisting(const GUIElement& element) const;
	/// <summary>
	/// Will calculate the element as if it is a root element
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	RenderInfo TryCalculateElementRenderInfoFromRoot(const GUIElement& element);

	void UpdateAll(const float deltaTime);

	void RenderAll();
	std::string ToStringTree() const;
};

