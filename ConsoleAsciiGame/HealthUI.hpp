#pragma once
#include <vector>
#include "UIObjectData.hpp"

namespace ECS
{
	class Entity;
}
class GameState;
class GlobalEntityManager;

namespace Game
{
	namespace UI
	{
		class HealthUI
		{
		private:
			std::vector<ECS::Entity*> m_health;
		public:

		private:
		public:
			HealthUI();

			void Init(GlobalEntityManager& globalEntities, GameState& state);
		};
	}
}
