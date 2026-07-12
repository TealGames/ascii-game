#pragma once
#include "ECS/Component/Types/World/PlayerComponent.hpp"
#include "Core/Input/InputManager.hpp"

#define ALLOW_PLAYER_CHEATS

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::Player
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

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
		void SetCheatStatus(const bool& enableCheats);
	};
}


