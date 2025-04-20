#include "pch.hpp"
#include "GameManager.hpp"

GameManager::GameManager(GlobalEntityManager& globalManager) 
	: m_state(), m_uiManager(globalManager, m_state) {}

void GameManager::GameValidate()
{
	m_uiManager.ValidateUI();
}

void GameManager::GameStart()
{
	m_uiManager.UpdateUI();
}

void GameManager::GameUpdate()
{
	m_uiManager.UpdateUI();
}