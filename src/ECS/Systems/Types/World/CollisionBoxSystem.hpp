#pragma once
#include "Core/Collision/CollisionRegistry.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"

class Scene;
class CameraComponent;
namespace ECS
{
	class CollisionBoxSystem 
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
		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);

		const ColliderOutlineBuffer& GetColliderBuffer() const;
		ColliderOutlineBuffer& GetColliderBufferMutable();

		std::vector<CollisionBoxData*> FindBodiesContainingPos(Scene& scene, const WorldPosition2D& worldPos) const;
	};
}


