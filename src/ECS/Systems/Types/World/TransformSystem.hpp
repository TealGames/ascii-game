#pragma once

namespace Engine
{
	namespace Scenes { class Scene; }
	namespace Camera { class CameraComponent; }
	class TransformSystem
	{
	private:
	public:

	private:
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}


