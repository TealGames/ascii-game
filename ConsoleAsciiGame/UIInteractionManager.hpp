#pragma once
#include "UISelectableData.hpp"
#include "InputManager.hpp"
#include <map>
#include <optional>
#include "UIHierarchy.hpp"

class UIRendererData;
class UIInteractionManager
{
private:
	const Input::InputManager& m_inputManager;
	UIHierarchy& m_hierarchy;
	std::map<UILayer, std::vector<UISelectableData*>, std::greater<UILayer>> m_selectableLayers;
	std::unordered_map<size_t, const UIRendererData*> m_selectionEventBlockers;
	//TODO: for optimization reasons, this is slow
	//std::map<GUIEventPriority, SelectableGUI*, std::greater<GUIEventPriority>> m_selectables;

	/// <summary>
	/// The current object that is selected. 
	/// Requires a mouse release event to fire on this object in order for it to be selected
	/// </summary>
	UISelectableData* m_currentSelected;
	/// <summary>
	/// The current object that is being dragged. 
	/// Requires a mouse down event to fire on this object and the mouse to continue to be held down
	/// even if moved to a different object
	/// </summary>
	UISelectableData* m_currentDragged;
	/// <summary>
	/// The current object that is being hovered over.
	/// Requires the mouse to be over this object regardless of any other events occuring at the same time
	/// </summary>
	UISelectableData* m_currentHovered;

	/// <summary>
	/// True if the current selectable was selected THIS frame
	/// </summary>
	bool m_selectedThisFrame;
	bool m_hasGuiTreeUpdated;
	Vec2 m_lastFrameMousePos;
public:

private:
	void TryUpdateTree();
	void InvokeInteractionEvents();
	void CreateSelectableArray();

	void SelectNewSelectable(UISelectableData* selectable);
	void DeselectCurrentSelectable();
	void ClickSelectable(UISelectableData* selectable);
	void StopCurrentHovering();
	void SetNewHoveredSelectable(UISelectableData* selectable);

	std::string ToStringSelectableTypes() const;
	
	bool IsEventBlocker(const size_t index) const;
	void AddSelectionEventBlocker(const UILayer layer, const size_t elementIndex, const UIRendererData& element);
public:
	UIInteractionManager(const Input::InputManager& input, UIHierarchy& hierarchy);

	void Init();
	void Update();

	void AddSelectable(const UILayer layer, UISelectableData* selectable);
	void AddSelectables(const UILayer layer, const std::vector<UISelectableData*>& selectables);

	bool SelectedSelectableThisFrame() const;
	bool HasSelecatbleSelected() const;
	const UISelectableData* TryGetSelectableSelected() const;
	const UISelectableData* TryGetSelectableDragged() const;
	const UISelectableData* TryGetSelectableHovered() const;

	std::string ToStringSelectables() const;
};

