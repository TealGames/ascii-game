#pragma once
#include <optional>
#include <vector>
#include "Utils/Math/Vec2Type.hpp"
#include "ECS/Entity/Entity.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Utils/Math/WorldPosition.hpp"
#include "Utils/Math/ScreenPosition.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"
#include "Core/Rendering/LineBuffer.hpp"

namespace Rendering { class Renderer; }
namespace ECS
{
	class CameraSystem
	{
	private:
		Rendering::Renderer* m_renderer;
		//FragmentedTextBuffer m_currentFrameBuffer;
		//ColliderOutlineBuffer* m_colliderOutlineBuffer;
		//LineBuffer* m_lineBuffer;

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
		//Camera(Transform& transform, const Vec2Int& viewportSize);

		void UpdateCameraPosition(CameraComponent& cameraData);

		void SystemUpdate(Scene& scene, CameraComponent& component, const float& deltaTime);

		/*const FragmentedTextBuffer& GetCurrentFrameBuffer() const;
		const ColliderOutlineBuffer* GetCurrentColliderOutlineBuffer() const;
		const LineBuffer* GetCurrentLineBuffer() const;*/
	};
}