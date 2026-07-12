#pragma once

namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	class UISliderSystem
	{
	private:
		const Input::InputManager* m_inputManager;
	public:

	private:
	public:
		UISliderSystem(const Input::InputManager& inputManager);
		void Init();
	};
}


