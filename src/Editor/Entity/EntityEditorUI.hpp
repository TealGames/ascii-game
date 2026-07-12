#pragma once
#include <optional>
#include "Editor/Entity/EntityUI.hpp"
#include "Core/Input/InputManager.hpp"


namespace Engine::UI { class PopupUIManager; class UIHierarchy;}
namespace Engine::Camera { class CameraComponent; class CameraController; }
namespace Engine::Assets { class AssetManager; };
namespace Engine::Editor::UI
{
	namespace MainUI = Engine::UI;
	class EntityEditorUI
	{
	private:
		const Input::InputManager* m_inputManager;
		Assets::AssetManager* m_assetManager;
		MainUI::PopupUIManager* m_popupManager;
		MainUI::UIHierarchy* m_guiTree;

		/*EntityGUICollection m_entityGUIs;
		EntityGUICollection::iterator m_selectedEntity;*/
		EntityUI m_selectedEntityUI;
	public:

	private:
		const Input::InputManager& GetInputManagerSafe() const;
		
	public:
		EntityEditorUI(const Input::InputManager& input, const Camera::CameraController& cameraController,
			MainUI::UIHierarchy& hierarchy, MainUI::PopupUIManager& popupManager, Assets::AssetManager& assetManager);
		~EntityEditorUI();

		void Init(ECS::EntityData& parent);

		void SetEntityUI(ECS::EntityData& entity);
		bool HasEntitySelected() const;
		void CloseCurrentEntityGUI();

		void Update();
		//void TryRender();
		//RenderInfo Render(const RenderInfo& renderInfo) override;
	};
}


