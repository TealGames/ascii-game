#pragma once
#include <optional>
#include <vector>
#include "Core/Primitives/Vector.hpp"
#include "ECS/Entity/Entity.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Primitives/ScreenPosition.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"
#include "Core/Rendering/LineBuffer.hpp"

namespace Engine::Rendering { class Renderer; }
namespace Engine::Scenes { class Scene; }
namespace Engine::Camera
{
	class CameraSystem
	{
	private:
		Rendering::Renderer* m_renderer;

	public:

	private:
		//void CollapseLayersWithinViewport(const Scene& scene, CameraComponent& cameraData);

		//bool IsWithinViewport(const TransformData& camera, const WorldPosition& pos) const;

	public:
		/// <summary>
		/// Creates a camera with mutli settings allowing for it to convert the data to a single output buffer
		/// which can be used to render to the screen
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="sceneManager">Manager needed to retrieve viewed data</param>
		/// <param name="followTarget">The object that that camera attempts to follow</param>
		/// <param name="viewportSize">THe amount of text vewied in WIDTH, HEIGHT</param>
		CameraSystem(Rendering::Renderer& renderer);

		void UpdateCameraPosition(CameraComponent& cameraData);
		void SystemUpdate(Scenes::Scene& scene, CameraComponent& component, const float& deltaTime);
	};
}