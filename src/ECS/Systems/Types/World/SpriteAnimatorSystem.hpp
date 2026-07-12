#pragma once

namespace Engine::ECS { class EntityData; }
namespace Engine::Rendering { class EntityRenderer2DSystem; }
namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::Animation
{
	class SpriteAnimation;
	class SpriteAnimatorSystem
	{
	private:
		Rendering::EntityRenderer2DSystem& m_EntityRenderer;
	public:

	private:
		void SetVisual(ECS::EntityData& entity, const SpriteAnimation& animation) const;
	public:
		SpriteAnimatorSystem(Rendering::EntityRenderer2DSystem& entityRenderer);
		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}
