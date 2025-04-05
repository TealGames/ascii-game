#pragma once
#include "MultiBodySystem.hpp" 
#include "CollisionRegistry.hpp"

namespace ECS
{
	class CollisionBoxSystem : public MultiBodySystem
	{
	private:
		CollisionRegistry& m_collisionRegistry;
	public:

	private:
		bool TryAddCollisionToRegistry(CollisionBoxData& boxA, 
			CollisionBoxData& boxB, const AABBIntersectionData& intersection);

	public:
		CollisionBoxSystem(CollisionRegistry& registry);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};
}


