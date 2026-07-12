#pragma once
#include <string>
#include <vector>
#include "ECS/Component/Component.hpp"
#include "Editor/Entity/ComponentFieldUI.hpp"
#include "Core/Input/InputManager.hpp"

namespace Engine::UI
{
	class UIInteractionManager;
	class PopupUIManager;
	class UIToggleComponent;
	class UITextComponent;
	class UILayoutComponent;
	class UIPanelComponent;
}
namespace Engine::Assets { class AssetManager; };
namespace Engine::Editor::UI
{
	namespace UI = Engine::UI;
	class EntityUI;
	class ComponentUI //: public ITreeGUIConstructible
	{
	private:
		const Input::InputManager* m_inputManager;
		UI::PopupUIManager* m_popupManager;

		UI::UITransformComponent* m_container;
		UI::UILayoutComponent* m_fieldLayout;
		UI::UIPanelComponent* m_nameHeader;
		ECS::Component* m_component;
		std::vector<ComponentFieldUI> m_fieldGUIs;

		UI::UIToggleComponent* m_dropdownCheckbox;
		UI::UITextComponent* m_componentNameText;

		const EntityUI* m_entityGUI;
	public:

	private:
		const Input::InputManager& GetInputManager() const;

	public:
		ComponentUI(const Input::InputManager& inputManager, UI::PopupUIManager& popupManager,
			Assets::AssetManager& m_assetManager, const EntityUI& entityGUI, UI::UILayoutComponent& parent);
		~ComponentUI();

		//void Init();
		void SetComponent(ECS::Component& component);
		void Update();
		//ScreenPosition Render(const RenderInfo& renderInfo) override;
		//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override; 

		std::string GetComponentName() const;
		std::vector<std::string> GetFieldNames() const;
		const std::vector<ComponentFieldUI>& GetFields() const;

		const EntityUI& GetEntityGUISafe() const;
	};
}


