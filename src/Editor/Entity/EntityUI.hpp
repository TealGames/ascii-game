#pragma once
#include <vector>	
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Editor/Entity/ComponentUI.hpp"

namespace Engine::UI
{
	class UIHierarchy;
	class PopupUIManager;
	class UILayoutComponent;
	class UIPanelComponent;
	class UIToggleComponent;
	class UITextComponent;
}
namespace Engine::Assets { class AssetManager; };
namespace Engine::Input { class InputManager; }
namespace Engine::Editor::UI
{
	namespace UI = Engine::UI;
	class EntityUI //: public ITreeGUIConstructible
	{
	private:
		const Input::InputManager* m_inputManager;
		UI::PopupUIManager* m_popupManager;
		Assets::AssetManager* m_assetManager;

		ECS::EntityData* m_layoutParent;
		UI::UILayoutComponent* m_guiLayout;
		UI::UIPanelComponent* m_entityHeader;
		ECS::EntityData* m_entity;

		std::vector<ComponentUI> m_componentUIs;
		UI::UITextComponent* m_entityNameText;
		UI::UIToggleComponent* m_activeToggle;
	public:

	private:
		void CreateLayout();
	public:
		EntityUI(const Input::InputManager& manager, UI::PopupUIManager& popupManager,
			Assets::AssetManager& assetManager);
		~EntityUI();

		void Init(ECS::EntityData& parent);

		void Update();
		void SetEntity(ECS::EntityData& entity);
		bool HasEntity() const;
		void ClearEntity();

		const ECS::EntityData& GetEntity() const;
	};
}


