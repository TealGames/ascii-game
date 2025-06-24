#pragma once
#include "Game/HealthUI.hpp"

class UIHierarchy;
namespace Game
{
	namespace UI
	{
		class GameUIManager
		{
		private:
			UIHierarchy& m_hierarchy;
			GameState& m_state;
			HealthUI m_healthUI;

		public:
		
		private:
		public:
			GameUIManager(UIHierarchy& hierarchy, GameState& state);

			void ValidateUI();
			void StartUI();
			void UpdateUI();
		};
	}
}


