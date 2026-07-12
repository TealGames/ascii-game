#pragma once
#include <functional>
#include <string>
#include "ECS/Component/ComponentField.hpp"

namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	class UIInteractionManager;
	class PopupUIManager;
	class UITextComponent;
	class UITransformComponent;
	class UILayoutComponent;
}
namespace Engine::Editor::UI
{
	namespace MainUI = Engine::UI;
	class ComponentUI;
	class ComponentFieldUI
	{
	private:
		const Input::InputManager* m_inputManager;
		MainUI::UITransformComponent* m_guiLayout;
		ECS::ComponentField* m_fieldInfo;
		std::vector<void*> m_fields;

		const ComponentUI* m_componentGUI;
		MainUI::UITextComponent* m_fieldNameText;
	public:

	private:
		void SetupInputFields(MainUI::UITransformComponent& fieldNameTextTransform, const bool fieldsStartNewLine);
		const Input::InputManager& GetInputManager() const;
		ECS::ComponentField& GetFieldInfo();

	public:
		ComponentFieldUI(const Input::InputManager& inputManager, MainUI::PopupUIManager& popupManager,
			const ComponentUI& componentGUI, MainUI::UILayoutComponent& parent);
		ComponentFieldUI(const ComponentFieldUI&) = default;
		~ComponentFieldUI();

		//void Init();
		void SetField(ECS::ComponentField& field);
		void Update();
		void SetFieldToInternal();
		void SetInternalWithInput();
		//ScreenPosition Render(const RenderInfo& renderInfo) override;
		//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;

		const ECS::ComponentField& GetFieldInfo() const;
		const ComponentUI& GetComponentGUISafe() const;
	};
}


