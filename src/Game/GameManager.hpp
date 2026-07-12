#pragma once
#include "Game/GameState.hpp"
#include "Game/GameUIManager.hpp"

namespace Engine::UI { class UIHierarchy; }
class GameManager
{
private:
	GameState m_state;
	GameUIManager m_uiManager;
public:

private:
public:
	GameManager(Engine::UI::UIHierarchy& hierarhcy);

	void GameValidate();
	void GameStart();
	void GameUpdate();
};

