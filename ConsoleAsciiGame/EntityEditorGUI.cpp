#include "pch.hpp"
#include "EntityEditorGUI.hpp"
#include "Globals.hpp"
#include "NormalizedPosition.hpp"
#include "PositionConversions.hpp"
#include "CameraController.hpp"

const Color EntityEditorGUI::EDITOR_TEXT_COLOR = WHITE;
const Color EntityEditorGUI::EDITOR_BACKGROUND_COLOR = { 30, 30, 30, 255 };
const Color EntityEditorGUI::EDITOR_SECONDARY_COLOR = GRAY;
const Color EntityEditorGUI::EDITOR_PRIMARY_COLOR = DARKGRAY;

const Vec2 EntityEditorGUI::EDITOR_CHAR_SPACING = { 3, 2 };

static const NormalizedPosition TOP_LEFT_POS_NORMALIZED = {0.8, 1};

EntityEditorGUI::EntityEditorGUI(const Input::InputManager& input, 
	const CameraController& cameraController, GUISelectorManager& selector)
	: m_inputManager(&input), m_selectorManager(&selector),
	m_defaultRenderInfo(), 
	m_selectedEntity(std::nullopt)
	//m_entityGUIs(), m_selectedEntity(m_entityGUIs.end())
{
	ScreenPosition topLeftPos = Conversions::NormalizedScreenToPosition(TOP_LEFT_POS_NORMALIZED);
	ScreenPosition topRightPos = Conversions::NormalizedScreenToPosition({ 1, 1 });
	m_defaultRenderInfo = RenderInfo(topLeftPos, ScreenPosition{ topRightPos.m_X - topLeftPos.m_X, SCREEN_HEIGHT });
}

const Input::InputManager& EntityEditorGUI::GetInputManagerSafe() const
{
	if (!Assert(this, m_inputManager != nullptr, "Tried to get input manager but is NULL"))
		throw std::invalid_argument("Invalid input manager state");

	return *m_inputManager;
}
GUISelectorManager& EntityEditorGUI::GetGUISelector()
{
	if (!Assert(this, m_inputManager != nullptr, "Tried to get gui selector manager but is NULL"))
		throw std::invalid_argument("Invalid gui selector manager state");

	return *m_selectorManager;
}

void EntityEditorGUI::SetEntityGUI(ECS::Entity& entity)
{
	if (m_selectedEntity.has_value() && m_selectedEntity.value().GetEntity() == entity) 
		return;

	m_selectedEntity = EntityGUI(GetInputManagerSafe(), GetGUISelector(), entity);
	m_selectedEntity.value().SetComponentsToStored();

	/*EntityGUICollection::iterator it = m_entityGUIs.find(entity.GetName());
	if (it == m_entityGUIs.end())
	{
		it = m_entityGUIs.emplace(entity.GetName(), EntityGUI(m_inputManager, m_selectorManager, entity)).first;
	}

	m_selectedEntity = it;
	m_selectedEntity->second.SetComponentsToStored();*/
}

void EntityEditorGUI::Update()
{
	if (m_selectedEntity.has_value()) 
		m_selectedEntity.value().Update();
}

void EntityEditorGUI::TryRender()
{
	//if (m_selectedEntity == m_entityGUIs.end()) return;
	Render(m_defaultRenderInfo);
}

ScreenPosition EntityEditorGUI::Render(const RenderInfo& renderInfo)
{
	if (!m_selectedEntity.has_value()) return {};
	m_selectedEntity.value().Render(renderInfo);

	return renderInfo.m_RenderSize;
}

