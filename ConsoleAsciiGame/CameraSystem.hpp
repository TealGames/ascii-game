#pragma once
#include <optional>
#include <vector>
#include "SingleBodySystem.hpp"
#include "Point2DInt.hpp"
#include "Entity.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "TransformSystem.hpp"
#include "RenderLayer.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "CameraData.hpp"

namespace ECS
{
	class CameraSystem : public SingleBodySystem<CameraData>
	{
	private:
		TransformSystem& m_transformSystem;
		std::optional<TextBuffer> m_currentFrameBuffer;

	public:

	private:
		TextBuffer CollapseLayersWithinViewport(const Scene& scene, CameraData& cameraData, ECS::Entity& mainCamera) const;

	public:
		/// <summary>
		/// Creates a camera with mutli settings allowing for it to convert the data to a single output buffer
		/// which can be used to render to the screen
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="sceneManager">Manager needed to retrieve viewed data</param>
		/// <param name="followTarget">The object that that camera attempts to follow</param>
		/// <param name="viewportSize">THe amount of text vewied in WIDTH, HEIGHT</param>
		CameraSystem(TransformSystem& transform);
		//Camera(Transform& transform, const Utils::Point2DInt& viewportSize);

		void UpdateCameraPosition(CameraData& cameraData, ECS::Entity& entity);

		void SystemUpdate(Scene& scene, CameraData& component,
			ECS::Entity& entity, const float& deltaTime) override;

		const TextBuffer* GetCurrentFrameBuffer() const;
	};
}