#pragma once
#include "VisualData.hpp"
#include "MultiBodySystem.hpp"
#include "SpriteAnimatorData.hpp"
#include "Entity.hpp"
#include "EntityRendererSystem.hpp"

namespace ECS
{
	class SpriteAnimatorSystem : MultiBodySystem
	{
	private:
		EntityRendererSystem& m_EntityRenderer;
	public:

	private:
		void SetVisual(ECS::Entity& entity, const SpriteAnimation& animation);
	public:
		SpriteAnimatorSystem(EntityRendererSystem& entityRenderer);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}
