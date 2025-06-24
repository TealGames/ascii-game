#pragma once
#include "GameState.hpp"
#include "GameUIManager.hpp"

class GlobalEntityManager;
class GameManager
{
private:
	GameState m_state;
	
	Game::UI::GameUIManager m_uiManager;
public:

private:
public:
	GameManager(GlobalEntityManager& globalManager);

	void GameValidate();
	void GameStart();
	void GameUpdate();
};

