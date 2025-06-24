#pragma once
#include "HealthUI.hpp"

class GlobalEntityManager;
namespace Game
{
	namespace UI
	{
		class GameUIManager
		{
		private:
			GlobalEntityManager& m_globalEntityManager;
			GameState& m_state;
			HealthUI m_healthUI;

		public:
		
		private:
		public:
			GameUIManager(GlobalEntityManager& globalManager, GameState& state);

			void ValidateUI();
			void StartUI();
			void UpdateUI();
		};
	}
}


