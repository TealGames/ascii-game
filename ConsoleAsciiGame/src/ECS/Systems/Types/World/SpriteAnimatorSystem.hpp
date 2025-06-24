#pragma once
#include "Core/Visual/VisualData.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorData.hpp"
#include "ECS/Systems/Types/World/EntityRendererSystem.hpp"

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
