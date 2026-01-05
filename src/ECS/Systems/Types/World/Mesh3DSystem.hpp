#pragma once

class Scene;
class CameraComponent;
class Mesh3DComponent;
class EngineState;
namespace Rendering { class Renderer; }
namespace ECS
{
	class Mesh3DSystem
	{
	private:
		Rendering::Renderer* m_renderer;
		const EngineState* m_engineState;
	public:

	private:
		void ComponentUpdate(Mesh3DComponent& component);
	public:
		Mesh3DSystem(Rendering::Renderer& renderer, const EngineState& engineState);
		void SystemStart(Scene& scene);
		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
	};
}