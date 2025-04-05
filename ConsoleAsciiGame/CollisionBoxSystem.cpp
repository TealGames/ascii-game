#include "pch.hpp"
#include "CollisionBoxSystem.hpp"
#include "Scene.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	CollisionBoxSystem::CollisionBoxSystem(CollisionRegistry& registry) : m_collisionRegistry(registry) {}

	void CollisionBoxSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("CollisionBoxSystem::SystemUpdate");
#endif 
		AABBIntersectionData collision = {};
		Vec2 minBodyDisplacement = {};
		Vec2 minBodyDisplacementVec = {};

		scene.OperateOnComponents<CollisionBoxData>(
			[&](CollisionBoxData& boxA, ECS::Entity& entityA)-> void
			{
				collision = {};
				minBodyDisplacement = {};
				minBodyDisplacementVec = {};

				//TODO: improve the searching algorithm by adding a broad phase and narrow phase such as by having a 
				// Bounding Volume Hierarhy for the narrow phase to reduce search time
				scene.OperateOnComponents<CollisionBoxData>(
					[&](CollisionBoxData& boxB, ECS::Entity& entityB)-> void
					{
						if (entityA == entityB) return;

						bool hasThisCollision = m_collisionRegistry.HasCollision(boxA, boxB);

						//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
						//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
						//collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
						collision = boxB.GetCollisionIntersectionData(boxA);
						//Assert(false, std::format("Checking collision"));
						if (collision.m_DoIntersect)
						{ 
							Assert(false, std::format("Found intersecting collision"));
							if (!hasThisCollision)
							{
								Assert(this, TryAddCollisionToRegistry(boxA, boxB, collision),
									std::format("Attempted to add collision to collision box system but something went wrong"));
							}
							return;
						}

						minBodyDisplacement = boxA.GetAABBMinDisplacement(boxB);
						minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
						//If we pass intersect check (meaning we didnt intersect) and it was valid we can remove iterator if we pass
						//the threshold in order to allow for some degree of touching to still be considered as colliding
						if (hasThisCollision && minBodyDisplacementVec.GetMagnitude() > CollisionBoxData::MAX_DISTANCE_FOR_COLLISION)
						{
							//boxA.RemoveCollidingBody(entityACollisionBIt);
							m_collisionRegistry.TryRemoveCollision(boxA, boxB);
						}
					});
			});
	}

	bool CollisionBoxSystem::TryAddCollisionToRegistry(CollisionBoxData& boxA, CollisionBoxData& boxB, const AABBIntersectionData& intersection)
	{
		Vec2 collidingDir = boxA.GetAABBDirection(boxB, true);
		std::optional<MoveDirection> maybeDirType = TryConvertVectorToDirection(collidingDir);
		if (!Assert(this, maybeDirType.has_value(), std::format("Tried to add collision between boxA:{} (entity:{}) and boxB:{} (entity:{}) "
			"but failed to get convert colliding dir:{} to direction type",
			boxA.ToString(), boxA.GetEntitySafe().ToString(), boxB.ToString(), boxB.GetEntitySafe().ToString(), collidingDir.ToString())))
		{
			return false;
		}

		return m_collisionRegistry.TryAddCollision(CollisionPair(boxA, boxB, intersection, maybeDirType.value()));
	}
}
