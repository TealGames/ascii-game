#pragma once
#include "ECS/Component/Types/World/PlayerData.hpp"
#include "Core/Input/InputManager.hpp"

#define ALLOW_PLAYER_CHEATS

class Scene;
class CameraComponent;
namespace ECS
{
	class PlayerSystem
	{
	private:
		bool m_lastFrameGrounded;
		bool m_cheatsEnabled;

		Input::InputManager& m_inputManager;
	public:

	private:

	public:
		PlayerSystem(Input::InputManager& input);

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
		void SetCheatStatus(const bool& enableCheats);
	};
}


