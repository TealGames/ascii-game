#pragma once
#include "Game/HealthUI.hpp"

namespace Engine::UI { class UIHierarchy; }
class GameUIManager
{
private:
	Engine::UI::UIHierarchy& m_hierarchy;
	GameState& m_state;
	HealthUI m_healthUI;

public:

private:
public:
	GameUIManager(Engine::UI::UIHierarchy& hierarchy, GameState& state);

	void ValidateUI();
	void StartUI();
	void UpdateUI();
};


