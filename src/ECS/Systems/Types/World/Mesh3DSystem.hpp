#pragma once

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::Core { class EngineState; }
namespace Engine::Rendering
{
	class Renderer;
	class Mesh3DComponent;
	class Mesh3DSystem
	{
	private:
		Rendering::Renderer* m_renderer;
		const Core::EngineState* m_engineState;
	public:

	private:
		void ComponentUpdate(Mesh3DComponent& component);
	public:
		Mesh3DSystem(Rendering::Renderer& renderer, const Core::EngineState& engineState);
		void SystemStart(Scenes::Scene& scene);
		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}