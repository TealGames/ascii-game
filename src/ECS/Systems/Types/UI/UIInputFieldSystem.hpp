#pragma once

namespace Engine::Scenes { class GlobalEntityManager; }
namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	class UIInputFieldSystem
	{
	private:
		const Input::InputManager* m_inputManager;
	public:

	private:
	public:
		UIInputFieldSystem(const Input::InputManager& input);
		void Init();
		void SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime);
	};
}


