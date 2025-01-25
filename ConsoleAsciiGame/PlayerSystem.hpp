#pragma once
#include "SingleBodySystem.hpp"
#include "PlayerData.hpp"

namespace ECS
{
	class PlayerSystem : public SingleBodySystem<PlayerData>
	{
	private:
	public:

	private:
	public:
		PlayerSystem();

		void SystemUpdate(Scene& scene, PlayerData& component, 
			ECS::Entity& entity, const float& deltaTime) override;
	};
}


