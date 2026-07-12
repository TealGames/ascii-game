#pragma once

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::World
{
	class TriggerSystem
	{
	private:
	public:

	private:
	public:
		TriggerSystem();

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}


