#pragma once
#include <optional>
#include <vector>
#include "ECS/Systems/SingleBodySystem.hpp"
#include "Utils/Data/Vec2Int.hpp"
#include "ECS/Entity/Entity.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/ScreenPosition.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"
#include "Core/Rendering/LineBuffer.hpp"

namespace Rendering { class Renderer; }
namespace ECS
{
	class CameraSystem : public SingleBodySystem<CameraData>
	{
	private:
		Rendering::Renderer* m_renderer;
		//FragmentedTextBuffer m_currentFrameBuffer;
		//ColliderOutlineBuffer* m_colliderOutlineBuffer;
		//LineBuffer* m_lineBuffer;

	public:

	private:
		void CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData);

		bool IsWithinViewport(const CameraData& camera, const WorldPosition& pos) const;

	public:
		/// <summary>
		/// Creates a camera with mutli settings allowing for it to convert the data to a single output buffer
		/// which can be used to render to the screen
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="sceneManager">Manager needed to retrieve viewed data</param>
		/// <param name="followTarget">The object that that camera attempts to follow</param>
		/// <param name="viewportSize">THe amount of text vewied in WIDTH, HEIGHT</param>
		CameraSystem(Rendering::Renderer& renderer, ColliderOutlineBuffer* colliderBuffer, LineBuffer* lineBuffer);
		//Camera(Transform& transform, const Vec2Int& viewportSize);

		void UpdateCameraPosition(CameraData& cameraData);

		void SystemUpdate(Scene& scene, CameraData& component, const float& deltaTime) override;

		/*const FragmentedTextBuffer& GetCurrentFrameBuffer() const;
		const ColliderOutlineBuffer* GetCurrentColliderOutlineBuffer() const;
		const LineBuffer* GetCurrentLineBuffer() const;*/
	};
}