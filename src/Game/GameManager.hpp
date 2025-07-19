#pragma once
#include "Game/GameState.hpp"
#include "Game/GameUIManager.hpp"

class UIHierarchy;
class GameManager
{
private:
	GameState m_state;
	
	Game::UI::GameUIManager m_uiManager;
public:

private:
public:
	GameManager(UIHierarchy& hierarhcy);

	void GameValidate();
	void GameStart();
	void GameUpdate();
};

