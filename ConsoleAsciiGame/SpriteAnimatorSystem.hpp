#pragma once
#include "VisualData.hpp"
#include "MultiBodySystem.hpp"
#include "SpriteAnimatorData.hpp"
#include "EntityRendererSystem.hpp"

class EntityData;
namespace ECS
{
	class SpriteAnimatorSystem : MultiBodySystem
	{
	private:
		EntityRendererSystem& m_EntityRenderer;
	public:

	private:
		void SetVisual(EntityData& entity, const SpriteAnimation& animation) const;
	public:
		SpriteAnimatorSystem(EntityRendererSystem& entityRenderer);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}
