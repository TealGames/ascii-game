#pragma once

namespace Engine::Scenes { class GlobalEntityManager; }
namespace Engine::UI
{
	class UIButtonSystem
	{
	private:
	public:

	private:
	public:
		UIButtonSystem();
		void Init();
		void SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime);
	};
}


