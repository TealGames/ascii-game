#pragma once
#include "Core/Visual/VisualData.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorData.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"

class EntityData;
namespace ECS
{
	class SpriteAnimatorSystem
	{
	private:
		EntityRenderer2DSystem& m_EntityRenderer;
	public:

	private:
		void SetVisual(EntityData& entity, const SpriteAnimation& animation) const;
	public:
		SpriteAnimatorSystem(EntityRenderer2DSystem& entityRenderer);
		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
	};
}
