#include "pch.hpp"
#include "EntityEditorGUI.hpp"
#include "Globals.hpp"
#include "NormalizedPosition.hpp"
#include "PositionConversions.hpp"
#include "CameraController.hpp"
#include "GUIHierarchy.hpp"

static const NormalizedPosition TOP_LEFT_POS_NORMALIZED = { 0.8, 1 };

EntityEditorGUI::EntityEditorGUI(const Input::InputManager& input,
	const CameraController& cameraController, GUIHierarchy& hiearchy, PopupGUIManager& popupManager)
	: m_inputManager(&input), m_guiTree(&hiearchy), m_popupManager(&popupManager),
	m_defaultRenderInfo(), 
	m_selectedEntity(nullptr)
	//m_entityGUIs(), m_selectedEntity(m_entityGUIs.end())
{
	
}
EntityEditorGUI::~EntityEditorGUI()
{
	TryCloseCurrentEntityGUI();
}

const Input::InputManager& EntityEditorGUI::GetInputManagerSafe() const
{
	if (!Assert(this, m_inputManager != nullptr, "Tried to get input manager but is NULL"))
		throw std::invalid_argument("Invalid input manager state");

	return *m_inputManager;
}

void EntityEditorGUI::SetEntityGUI(ECS::Entity& entity)
{
	if (HasEntitySelected() && m_selectedEntity->GetEntity() == entity)
		return;

	//TODO: we should not be able to delete entity gui like this especially if it has gui elements,
	//therefore there must be some way to handle it without causing problems to the gui system
	TryCloseCurrentEntityGUI();
	m_selectedEntity = new EntityGUI(GetInputManagerSafe(), *m_popupManager, entity);
	m_selectedEntity->SetComponentsToStored();

	m_selectedEntity->GetTreeGUI()->SetBounds(TOP_LEFT_POS_NORMALIZED, NormalizedPosition::BOTTOM_RIGHT);
	m_guiTree->AddToRoot(DEFAULT_LAYER, m_selectedEntity->GetTreeGUI());
	//Assert(false, std::format("Clicked entity:{} tree:{}", m_selectedEntity->GetTreeGUI()->ToStringBase(), m_guiTree->ToStringTree()));
}
bool EntityEditorGUI::HasEntitySelected() const
{
	return m_selectedEntity != nullptr;
}
bool EntityEditorGUI::TryCloseCurrentEntityGUI()
{
	if (!HasEntitySelected()) return false;

	delete m_selectedEntity;
	m_selectedEntity = nullptr;
}

void EntityEditorGUI::Update()
{
	if (m_selectedEntity == nullptr) return;

	m_selectedEntity->Update();
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

