#pragma once

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine
{
	class TestSystem
	{
	private:
	public:

	private:
	public:
		TestSystem();

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& cam, const float& deltaTime);
	};
}