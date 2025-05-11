#pragma once
#include "ContainerGUI.hpp"
#include <cstdint>
#include <map>
#include <limits>
#include <functional>
#include "Event.hpp"

using GUILayer = std::uint8_t;
constexpr GUILayer TOP_LAYER = std::numeric_limits<GUILayer>::max();
constexpr GUILayer BOTTOM_LAYER = std::numeric_limits<GUILayer>::min();
constexpr GUILayer DEFAULT_LAYER = BOTTOM_LAYER;

using GUIElementLayersCollection = std::map<GUILayer, GUIElement*>;
class GUIHierarchy
{
private:
	Vec2Int m_rootSize;
	/// <summary>
	/// Stores each of the layer's gui element roots. Note: layers are stored in increasing priority order 
	/// (meaning background layers first, then foreground layers)
	/// </summary>
	GUIElementLayersCollection m_layerRoots;
	std::vector<ContainerGUI> m_rootElements;
public:
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
	GUIElementLayersCollection::iterator CreateNewLayer(const GUILayer layer);
	void ClearLayer(const GUILayer layer);

	bool IsLayerRootID(const GUIElementID id) const;

public:
	GUIHierarchy(const Vec2Int rootCanvasSize);

	void AddToRoot(const GUILayer layer, GUIElement* element);

	GUIElement* FindMutable(const GUIElementID id);
	GUIElement* FindParentMutable(const GUIElementID id, size_t* foundChildIndex =nullptr);
	std::vector<GUIElement*> GetLayerRootsMutable(const bool topLayerFirst);
	/// <summary>
	/// Will add the element as a child of the gui element with the id
	/// Note: will return false if the parent id is not found and true otherwise
	/// </summary>
	/// <param name="layer"></param>
	/// <param name="parentId"></param>
	/// <param name="element"></param>
	bool TryAddElementAsChild(const GUIElementID parentId, GUIElement* element);
	/// <summary>
	/// Will remove the element's children from the tree
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	GUIElement* TryRemoveElementChildren(const GUIElementID id);
	/// <summary>
	/// Will remove this element from the tree including all of its children
	/// Note: removing the root element of a layer is NOT allowed
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	GUIElement* TryRemoveElement(const GUIElementID id);

	/// <summary>
	/// Will execute the action on all elements (except each layer's root) 
	/// based on their order in the layer and going from top layer to bottom layer
	/// </summary>
	void ExecuteOnAllElementsDescending(const std::function<void(GUILayer, GUIElement*)>& action);

	void UpdateAll(const float deltaTime);
	void RenderAll();
};

