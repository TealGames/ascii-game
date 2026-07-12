#pragma once
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/Input/InputManager.hpp"
#include <map>
#include <set>
#include <optional>
#include "Core/UI/UIHierarchy.hpp"

namespace Engine::UI
{
	class UIInteractionManager
	{
	private:
		const Input::InputManager& m_inputManager;
		UIHierarchy& m_hierarchy;
		std::map<UILayer, std::vector<UISelectableComponent*>, std::greater<UILayer>> m_selectableLayers;
		std::unordered_map<size_t, const UITransformComponent*> m_selectionEventBlockers;
		std::set<ECS::EntityID> m_selectableIds;
		//TODO: for optimization reasons, this is slow
		//std::map<GUIEventPriority, SelectableGUI*, std::greater<GUIEventPriority>> m_selectables;

		/// <summary>
		/// The current object that is selected. 
		/// Requires a mouse release event to fire on this object in order for it to be selected
		/// </summary>
		UISelectableComponent* m_currentSelected;
		/// <summary>
		/// The current object that is being dragged. 
		/// Requires a mouse down event to fire on this object and the mouse to continue to be held down
		/// even if moved to a different object
		/// </summary>
		UISelectableComponent* m_currentDragged;
		/// <summary>
		/// The current object that is being hovered over.
		/// Requires the mouse to be over this object regardless of any other events occuring at the same time
		/// </summary>
		UISelectableComponent* m_currentHovered;

		/// <summary>
		/// True if the current selectable was selected THIS frame
		/// </summary>
		bool m_selectedThisFrame;
		Vec2 m_lastFrameMousePos;
	public:

	private:
		void InvokeInteractionEvents();
		void CreateSelectableArray();

		void SelectableSelectCallback(UISelectableComponent* selectable);
		void DeselectCurrentSelectable();
		void SelectableClickCallback(UISelectableComponent* selectable);
		void StopCurrentHovering();
		void SelectableHoverStartCallback(UISelectableComponent* selectable);
		void StartDrag(UISelectableComponent* selectable);
		void EndCurrentDrag();

		std::string ToStringSelectableTypes() const;

		bool IsEventBlocker(const size_t index) const;
		void AddSelectionEventBlocker(const UILayer layer, const size_t elementIndex, const UITransformComponent& element);
	public:
		UIInteractionManager(const Input::InputManager& input, UIHierarchy& hierarchy);

		void Update();

		void AddSelectable(const UILayer layer, UISelectableComponent* selectable);
		void AddSelectables(const UILayer layer, const std::vector<UISelectableComponent*>& selectables);

		bool SelectedSelectableThisFrame() const;
		bool HasSelectableSelected() const;
		bool HasSelectableHovered() const;
		bool HasSelectableDragged() const;
		const UISelectableComponent* TryGetSelectableSelected() const;
		const UISelectableComponent* TryGetSelectableDragged() const;
		const UISelectableComponent* TryGetSelectableHovered() const;

		std::string ToStringSelectables() const;
	};

}

