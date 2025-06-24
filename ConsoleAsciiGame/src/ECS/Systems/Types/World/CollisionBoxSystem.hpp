#pragma once
#include "MultiBodySystem.hpp" 
#include "CollisionRegistry.hpp"
#include "ColliderOutlineBuffer.hpp"

namespace ECS
{
	class CollisionBoxSystem : public MultiBodySystem
	{
	private:
		CollisionRegistry& m_collisionRegistry;
		ColliderOutlineBuffer m_colliderOutlineBuffer;

	public:

	private:
		bool TryAddCollisionToRegistry(CollisionBoxData& boxA, 
			CollisionBoxData& boxB, const AABBIntersectionData& intersection);

	public:
		CollisionBoxSystem(CollisionRegistry& registry);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;

		const ColliderOutlineBuffer& GetColliderBuffer() const;
		ColliderOutlineBuffer& GetColliderBufferMutable();

		std::vector<CollisionBoxData*> FindBodiesContainingPos(Scene& scene, const WorldPosition& worldPos) const;
	};
}


