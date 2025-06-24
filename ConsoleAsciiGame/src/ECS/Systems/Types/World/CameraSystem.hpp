#pragma once
#include <optional>
#include <vector>
#include "SingleBodySystem.hpp"
#include "Vec2Int.hpp"
#include "Entity.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "TransformSystem.hpp"
#include "RenderLayer.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "CameraData.hpp"
#include "WorldPosition.hpp"
#include "ScreenPosition.hpp"
#include "TextBuffer.hpp"
#include "ColliderOutlineBuffer.hpp"
#include "LineBuffer.hpp"

namespace ECS
{
	class CameraSystem : public SingleBodySystem<CameraData>
	{
	private:
		FragmentedTextBuffer m_currentFrameBuffer;
		ColliderOutlineBuffer* m_colliderOutlineBuffer;
		LineBuffer* m_lineBuffer;

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
		CameraSystem(ColliderOutlineBuffer* colliderBuffer, LineBuffer* lineBuffer);
		//Camera(Transform& transform, const Vec2Int& viewportSize);

		void UpdateCameraPosition(CameraData& cameraData);

		void SystemUpdate(Scene& scene, CameraData& component, const float& deltaTime) override;

		const FragmentedTextBuffer& GetCurrentFrameBuffer() const;
		const ColliderOutlineBuffer* GetCurrentColliderOutlineBuffer() const;
		const LineBuffer* GetCurrentLineBuffer() const;
	};
}