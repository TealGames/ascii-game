#pragma once
#include <optional>
#include "Point2DInt.hpp"
#include "Entity.hpp"
#include "Updateable.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "RenderLayer.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

namespace ECS
{
	class Camera : public Component
	{
	private:
		Transform& m_transform;
		const SceneManagement::SceneManager& m_sceneManager;

		Utils::Point2DInt m_viewportSize;

		const ECS::Entity* m_followTarget;
		std::optional<TextBuffer> m_lastFrameBuffer;

	public:

	private:
		TextBuffer CollapseLayersWithinViewport() const;

	public:
		/// <summary>
		/// Creates a camera with mutli settings allowing for it to convert the data to a single output buffer
		/// which can be used to render to the screen
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="sceneManager">Manager needed to retrieve viewed data</param>
		/// <param name="followTarget">The object that that camera attempts to follow</param>
		/// <param name="viewportSize">THe amount of text vewied in WIDTH, HEIGHT</param>
		Camera(Transform& transform, const SceneManagement::SceneManager& sceneManager, 
			ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize);
		//Camera(Transform& transform, const Utils::Point2DInt& viewportSize);

		void UpdateCameraPosition();
		UpdatePriority GetUpdatePriority() const override;

		void Start() override;
		void UpdateStart(float deltaTime) override;
		void UpdateEnd(float deltaTime) override;
	};
}