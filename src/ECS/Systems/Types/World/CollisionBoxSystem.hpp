#pragma once
#include "Core/Collision/CollisionRegistry.hpp"
#include "Core/Collision/ColliderOutlineBuffer.hpp"

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::Physics
{
	class CollisionBoxSystem 
	{
	private:
		CollisionRegistry& m_collisionRegistry;
		Rendering::ColliderOutlineBuffer m_colliderOutlineBuffer;

	public:

	private:
		bool TryAddCollisionToRegistry(CollisionBoxComponent& boxA, 
			CollisionBoxComponent& boxB, const AABBIntersectionData& intersection);

	public:
		CollisionBoxSystem(CollisionRegistry& registry);
		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);

		const Rendering::ColliderOutlineBuffer& GetColliderBuffer() const;
		Rendering::ColliderOutlineBuffer& GetColliderBufferMutable();

		std::vector<CollisionBoxComponent*> FindBodiesContainingPos(Scenes::Scene& scene, const WorldPosition2D& worldPos) const;
	};
}


