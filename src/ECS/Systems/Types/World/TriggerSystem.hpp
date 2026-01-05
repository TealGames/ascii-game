#pragma once

class Scene;
class CameraComponent;
namespace ECS
{
	class TriggerSystem
	{
	private:
	public:

	private:
	public:
		TriggerSystem();

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
	};
}


