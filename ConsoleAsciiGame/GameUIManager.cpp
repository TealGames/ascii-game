#include "pch.hpp"
#include "GameUIManager.hpp"
#include "GlobalEntityManager.hpp"

namespace Game
{
	namespace UI
	{
		GameUIManager::GameUIManager(GlobalEntityManager& globalManager, GameState& state)
			: m_globalEntityManager(globalManager), m_state(state), m_healthUI() {}

		void GameUIManager::ValidateUI()
		{

		}
		void GameUIManager::StartUI()
		{
			m_healthUI.Init(m_globalEntityManager, m_state);
		}
		void GameUIManager::UpdateUI()
		{

		}
	}
}
