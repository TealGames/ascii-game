#pragma once
#include <vector>
#include "UIObjectData.hpp"

class EntityData;
class GameState;
class GlobalEntityManager;

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

			void Init(GlobalEntityManager& globalEntities, GameState& state);
		};
	}
}
