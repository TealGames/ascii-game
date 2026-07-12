#include "pch.hpp"
#include "Game/GameUIManager.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"

using namespace Engine;

GameUIManager::GameUIManager(UI::UIHierarchy& hierarchy, GameState& state)
	: m_hierarchy(hierarchy), m_state(state), m_healthUI() {}

void GameUIManager::ValidateUI()
{

}
void GameUIManager::StartUI()
{
	m_healthUI.Init(m_hierarchy, m_state);
}
void GameUIManager::UpdateUI()
{

}
