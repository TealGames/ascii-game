#pragma once
#include "MultiBodySystem.hpp"
#include "PlayerData.hpp"
#include "InputManager.hpp"

#define ALLOW_PLAYER_CHEATS

namespace ECS
{
	class PlayerSystem : MultiBodySystem
	{
	private:
		bool m_lastFrameGrounded;
		bool m_cheatsEnabled;

		Input::InputManager& m_inputManager;
	public:

	private:

	public:
		PlayerSystem(Input::InputManager& input);

		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;

		void SetCheatStatus(const bool& enableCheats);
	};
}


