#pragma once
#include "SingleBodySystem.hpp"
#include "PlayerData.hpp"
#include "InputManager.hpp"

#define ALLOW_PLAYER_CHEATS

namespace ECS
{
	class PlayerSystem : public SingleBodySystem<PlayerData>
	{
	private:
		bool m_lastFrameGrounded;
		bool m_cheatsEnabled;

		Input::InputManager& m_inputManager;
	public:

	private:

	public:
		PlayerSystem(Input::InputManager& input);

		void SystemUpdate(Scene& scene, PlayerData& component, 
			ECS::Entity& entity, const float& deltaTime) override;

		void SetCheatStatus(const bool& enableCheats);
	};
}


