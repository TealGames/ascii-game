#include "pch.hpp"
#include "EntityEditorGUI.hpp"
#include "Globals.hpp"
#include "NormalizedPosition.hpp"
#include "PositionConversions.hpp"
#include "CameraData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

static const NormalizedPosition TOP_LEFT_POS_NORMALIZED = {0.8, 1};

EntityEditorGUI::EntityEditorGUI(const Input::InputManager& input, const SceneManagement::SceneManager& scene, GUISelectorManager& selector)
	: m_inputManager(input), m_sceneManager(scene), m_selectorManager(selector), 
	m_entityGUIs(), m_selectedEntity(m_entityGUIs.end()), m_currentRenderInfo() {}

void EntityEditorGUI::SetEntityGUI(ECS::Entity& entity)
{
	EntityGUICollection::iterator it = m_entityGUIs.find(entity.m_Name);
	if (it == m_entityGUIs.end())
	{
		it = m_entityGUIs.emplace(entity.m_Name, EntityGUI(m_inputManager, m_selectorManager, entity)).first;
	}

	m_selectedEntity = it;
	m_selectedEntity->second.SetComponentsToStored();
}

void EntityEditorGUI::Update()
{
	if (m_inputManager.GetInputKey(MOUSE_BUTTON_LEFT)->GetState().IsDown())
	{
		ScreenPosition mouseClickedPos = m_inputManager.GetMousePosition();

		const CameraData* maybeCamera = m_sceneManager.GetActiveScene()->TryGetMainCamera();
		if (!Assert(this, maybeCamera != nullptr, std::format("Tried to update entity editor GUI "
			"with new entity clicked, but could not retreive camera!")))
			return;

		WorldPosition worldClickedPos = Conversions::ScreenToWorldPosition(*maybeCamera, mouseClickedPos);

		auto entitiesWithinPos = m_sceneManager.GetActiveScene()->GetPhysicsWorld().FindBodiesContainingPos(worldClickedPos);
		if (!entitiesWithinPos.empty())
		{
			ECS::Entity& selectedEntity = entitiesWithinPos[0]->GetEntitySafeMutable();
			if (m_selectedEntity == m_entityGUIs.end() || selectedEntity != m_selectedEntity->second.GetEntity())
			{
				SetEntityGUI(selectedEntity);
				Log(std::format("Successfully selected entity: '{}'", m_selectedEntity->second.GetEntity().m_Name), false, true);

				ScreenPosition topLeftPos = Conversions::NormalizedScreenToPosition(TOP_LEFT_POS_NORMALIZED);
				ScreenPosition topRightPos= Conversions::NormalizedScreenToPosition({1, 1});
				m_currentRenderInfo = RenderInfo(topLeftPos, topRightPos- topLeftPos);
			}
			//else Log(std::format("Failed to select any entity"), false, true);
		}
	}

	if (m_selectedEntity!= m_entityGUIs.end()) m_selectedEntity->second.Update();
}

void EntityEditorGUI::TryRender()
{
	if (m_selectedEntity == m_entityGUIs.end()) return;

	Render(m_currentRenderInfo);
}

ScreenPosition EntityEditorGUI::Render(const RenderInfo& renderInfo)
{
	if (m_selectedEntity == m_entityGUIs.end()) return {};

	m_selectedEntity->second.Render(renderInfo);
	//Assert(false, "REDNER");
	return renderInfo.m_RenderSize;
}

