#include "pch.hpp"
#include "EntityEditorUI.hpp"
#include "Globals.hpp"
#include "NormalizedPosition.hpp"
#include "PositionConversions.hpp"
#include "CameraController.hpp"
#include "UIHierarchy.hpp"

EntityEditorUI::EntityEditorUI(const Input::InputManager& input,
	const CameraController& cameraController, UIHierarchy& hiearchy, PopupUIManager& popupManager, const AssetManagement::AssetManager& assetManager, EntityData& parent)
	: m_inputManager(&input), m_guiTree(&hiearchy), m_popupManager(&popupManager), m_assetManager(&assetManager), m_selectedEntityUI(input, popupManager, assetManager, parent)
	//m_entityGUIs(), m_selectedEntity(m_entityGUIs.end())
{

}

EntityEditorUI::~EntityEditorUI()
{
	CloseCurrentEntityGUI();
}

const Input::InputManager& EntityEditorUI::GetInputManagerSafe() const
{
	if (!Assert(this, m_inputManager != nullptr, "Tried to get input manager but is NULL"))
		throw std::invalid_argument("Invalid input manager state");

	return *m_inputManager;
}

void EntityEditorUI::SetEntityUI(EntityData& entity)
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

//void EntityEditorGUI::TryRender()
//{
//	//if (m_selectedEntity == m_entityGUIs.end()) return;
//	ScreenPosition topLeftPos = Conversions::NormalizedScreenToPosition(TOP_LEFT_POS_NORMALIZED);
//	ScreenPosition topRightPos = Conversions::NormalizedScreenToPosition({ 1, 1 });
//	Render(RenderInfo(topLeftPos, ScreenPosition{ topRightPos.m_X - topLeftPos.m_X, SCREEN_HEIGHT }));
//}

/*
RenderInfo EntityEditorGUI::Render(const RenderInfo& renderInfo)
{
	if (m_selectedEntity==nullptr) return {};
	m_selectedEntity->Render(renderInfo);

	return renderInfo.m_RenderSize;
}
*/

