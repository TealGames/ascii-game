#include "pch.hpp"
#include "Game/GameManager.hpp"

using namespace Engine;

GameManager::GameManager(UI::UIHierarchy& hierarchy) 
	: m_state(), m_uiManager(hierarchy, m_state) {}

void GameManager::GameValidate()
{
	m_uiManager.ValidateUI();
}

void GameManager::GameStart()
{
	m_uiManager.StartUI();
}

void GameManager::GameUpdate()
{
	m_uiManager.UpdateUI();
}