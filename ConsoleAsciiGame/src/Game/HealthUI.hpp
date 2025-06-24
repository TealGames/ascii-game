#pragma once
#include <vector>

class EntityData;
class GameState;
class GlobalEntityManager;
class UIHierarchy;

namespace Game
{
	namespace UI
	{
		class HealthUI
		{
		private:
			std::vector<EntityData*> m_health;
		public:

		private:
		public:
			HealthUI();

			void Init(UIHierarchy& hierarchy, GameState& state);
		};
	}
}
