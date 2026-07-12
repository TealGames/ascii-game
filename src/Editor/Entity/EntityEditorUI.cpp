#include "pch.hpp"
#include "Editor/Entity/EntityEditorUI.hpp"
#include "StaticGlobals.hpp"
#include "Core/Primitives/NormalizedVec2.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Core/UI/UIHierarchy.hpp"

namespace Engine::Editor::UI
{
	EntityEditorUI::EntityEditorUI(const Input::InputManager& input,
		const Camera::CameraController& cameraController, MainUI::UIHierarchy& hiearchy, MainUI::PopupUIManager& popupManager, Assets::AssetManager& assetManager)
		: m_inputManager(&input), m_guiTree(&hiearchy), m_popupManager(&popupManager), m_assetManager(&assetManager), 
		m_selectedEntityUI(input, popupManager, assetManager)
		//m_entityGUIs(), m_selectedEntity(m_entityGUIs.end())
	{

	}

	EntityEditorUI::~EntityEditorUI()
	{
		CloseCurrentEntityGUI();
	}

	void EntityEditorUI::Init(ECS::EntityData& parent)
	{
		m_selectedEntityUI.Init(parent);
	}

	const Input::InputManager& EntityEditorUI::GetInputManagerSafe() const
	{
		if (!Assert(m_inputManager != nullptr, "Tried to get input manager but is NULL"))
			throw std::invalid_argument("Invalid input manager state");

		return *m_inputManager;
	}

	void EntityEditorUI::SetEntityUI(ECS::EntityData& entity)
	{
		if (HasEntitySelected() && m_selectedEntityUI.GetEntity() == entity)
			return;

		//TODO: we should not be able to delete entity gui like this especially if it has gui elements,
		//therefore there must be some way to handle it without causing problems to the gui system
		m_selectedEntityUI.SetEntity(entity);

		//m_selectedEntityUI->GetTreeGUI()->SetBounds(TOP_LEFT_POS_NORMALIZED, NormalizedPosition::BOTTOM_RIGHT);
		//m_guiTree->AddToRoot(DEFAULT_LAYER, m_selectedEntityUI->GetTreeGUI());
		//Assert(false, std::format("Clicked entity:{} tree:{}", m_selectedEntity->GetTreeGUI()->ToStringBase(), m_guiTree->ToStringTree()));
	}
	bool EntityEditorUI::HasEntitySelected() const
	{
		return m_selectedEntityUI.HasEntity();
	}
	void EntityEditorUI::CloseCurrentEntityGUI()
	{
		m_selectedEntityUI.ClearEntity();
	}

	void EntityEditorUI::Update()
	{
		if (!m_selectedEntityUI.HasEntity()) return;

		m_selectedEntityUI.Update();
	}
}

