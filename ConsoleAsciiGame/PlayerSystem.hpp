#pragma once
#include "SingleBodySystem.hpp"
#include "PlayerData.hpp"

namespace ECS
{
	class PlayerSystem : public SingleBodySystem<PlayerData>
	{
	private:
		bool m_isGroudnedLastFrame = false;
		int groundedTimes = 0;

		bool m_cheatsEnabled;
	public:

	private:
	public:
		PlayerSystem();

		void SystemUpdate(Scene& scene, PlayerData& component, 
			ECS::Entity& entity, const float& deltaTime) override;

		void SetCheatStatus(const bool& enableCheats);
	};
}


