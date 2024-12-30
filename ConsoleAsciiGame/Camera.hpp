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
	struct CameraSettings
	{
		const ECS::Entity* m_FollowTarget;
		bool m_HasFixedPosition;
		Utils::Point2DInt m_ViewportSize;

		/// <summary>
		/// Moveable camera that follows target
		/// </summary>
		/// <param name="followTarget"></param>
		CameraSettings(const ECS::Entity& followTarget, const Utils::Point2DInt& viewportSize);
		CameraSettings(const Utils::Point2DInt& viewportSize);
	};

	class Camera : public Component
	{
	private:
		Transform& m_transform;
		const SceneManagement::SceneManager& m_sceneManager;
		const CameraSettings m_cameraSettings;

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
			const CameraSettings& settings);
		//Camera(Transform& transform, const Utils::Point2DInt& viewportSize);

		void UpdateCameraPosition();
		UpdatePriority GetUpdatePriority() const override;

		void Start() override;
		void UpdateStart(float deltaTime) override;
		void UpdateEnd(float deltaTime) override;
	};
}