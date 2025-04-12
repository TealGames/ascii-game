#include "pch.hpp"
#include "CollisionBoxSystem.hpp"
#include "Scene.hpp"
#include "PositionConversions.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	static constexpr bool RENDER_COLLIDER_OUTLINES = true;

	CollisionBoxSystem::CollisionBoxSystem(CollisionRegistry& registry) : m_collisionRegistry(registry) {}

	void CollisionBoxSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("CollisionBoxSystem::SystemUpdate");
#endif 
		m_colliderOutlineBuffer.ClearAll();

		AABBIntersectionData collision = {};
		Vec2 minBodyDisplacement = {};
		Vec2 minBodyDisplacementVec = {};

		std::vector<std::string> bounds = {};
		int collisionsAdded = 0;

		m_collisionRegistry.ClearAll();
		scene.OperateOnComponents<CollisionBoxData>(
			[&](CollisionBoxData& boxA, ECS::Entity& entityA) -> void
			{
				collision = {};
				minBodyDisplacement = {};
				minBodyDisplacementVec = {};

				bounds.push_back(std::format("[ENTITY:{} BOX:{}]", entityA.GetName(), boxA.GetAABB().ToString(boxA.GetAABBCenterWorldPos())));
				if (RENDER_COLLIDER_OUTLINES)
				{
					/*if (!Assert(this, mainCamera != nullptr, std::format("Tried to render collider outlines for entity: {} "
						"but the scene:{} has no active camera!", entity->GetName(), scene.GetName()))) return;*/

						//WorldPosition topLeftColliderPos = body->GetAABBTopLeftWorldPos();
					WorldPosition topLeftColliderPos = boxA.GetAABBTopLeftWorldPos();
					//TODO: the camera should convert to screen pos not here
					ScreenPosition topLeftScreenPos = Conversions::WorldToScreenPosition(mainCamera, topLeftColliderPos);
					/*LogWarning(std::format("ADDING OUTLINE for entity: {} pos: {} top left collider: {} SCREEN TOP LEFT: {} half size: {}",
						entity.m_Name, entity.m_Transform.m_Pos.ToString(), topLeftColliderPos.ToString(), topLeftScreenPos.ToString(), body.GetAABB().GetHalfExtent().ToString()));*/

						//m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(body->GetAABB().GetSize(), topLeftScreenPos));
					m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(boxA.GetAABB().GetSize(), topLeftScreenPos));
				}

				//TODO: improve the searching algorithm by adding a broad phase and narrow phase such as by having a 
				// Bounding Volume Hierarhy for the narrow phase to reduce search time
				scene.OperateOnComponents<CollisionBoxData>(
					[&](CollisionBoxData& boxB, ECS::Entity& entityB) -> void
					{
						if (entityA == entityB) return;

						bool hasThisCollision = m_collisionRegistry.HasCollision(boxA, boxB);
						if (hasThisCollision) return;

						//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
						//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
						//collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());

						//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
						//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
						collision = boxA.GetCollisionIntersectionData(boxB);

						/*LogError(std::format("Found collisions: between{}({}) and {}({}) ->{}", boxA.ToString(), entityA.GetName(), 
							boxB.ToString(), entityB.GetName(), collision.ToString()));*/
						//Assert(false, std::format("Checking collision"));
						if (collision.m_DoIntersect)
						{ 
							if (std::abs(collision.m_Depth.m_X) <= CollisionBoxData::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_X = 0;
							if (std::abs(collision.m_Depth.m_Y) <= CollisionBoxData::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_Y = 0;

							/*if (!Assert(this, collision.m_Depth.m_X!=0 && collision.m_Depth.m_Y!=0, std::format("Tried to check collision between:{}({}) and {}({}) "
								"with intersecting from data TRUE but depth is 0:{}", boxA.ToString(), entityA.GetName(),
								boxB.ToString(), entityB.GetName(), collision.ToString())))
								return;*/

							//Assert(false, std::format("Found intersecting collision"));
							/*if (!hasThisCollision)
							{
								Assert(this, TryAddCollisionToRegistry(boxA, boxB, collision),
									std::format("Attempted to add collision to collision box system but something went wrong"));
								collisionsAdded++;
							}*/
							//return;

							TryAddCollisionToRegistry(boxA, boxB, collision);
						}

						/*
						//TODO: maybe we could work in min aabb displacement into depth of intersection data to prevent so many complex calculations
						minBodyDisplacement = boxA.GetAABBMinDisplacement(boxB);
						minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
						//If we pass intersect check (meaning we didnt intersect) and it was valid we can remove iterator if we pass
						//the threshold in order to allow for some degree of touching to still be considered as colliding
						if (hasThisCollision && minBodyDisplacementVec.GetMagnitude() > CollisionBoxData::MAX_DISTANCE_FOR_COLLISION)
						{
							//boxA.RemoveCollidingBody(entityACollisionBIt);
							//Assert(false, std::format("Reached max distance for collision and exited"));
							m_collisionRegistry.TryRemoveCollision(boxA, boxB);
							collisionsAdded--;
						}
						*/
					});
			});

		/*LogError(std::format("All colliders:{} ------ COLLISIONS:{} REGISTRY:{}", Utils::ToStringIterable<std::vector<std::string>, std::string>(bounds),
			std::to_string(collisionsAdded), m_collisionRegistry.ToStringCollidingBodies()));*/
	}

	bool CollisionBoxSystem::TryAddCollisionToRegistry(CollisionBoxData& boxA, CollisionBoxData& boxB, const AABBIntersectionData& intersection)
	{
		const Vec2 collidingDir = intersection.IsTouchingIntersection()? boxB.GetAABBDirection(boxA) : intersection.m_Depth.GetNormalized();
		

		if (intersection.IsTouchingIntersection())
		{
			//Assert(false, std::format("Found touching dir:{}", collidingDir.ToString()));
		}
		std::optional<MoveDirection> maybeDirType = TryConvertVectorToDirection(collidingDir);

		if (!Assert(this, maybeDirType.has_value(), std::format("Tried to add collision between boxA:{} (entity:{}) and boxB:{} (entity:{}) "
			"but failed to get convert colliding dir:{} to direction type",
			boxA.ToString(), boxA.GetEntitySafe().ToString(), boxB.ToString(), boxB.GetEntitySafe().ToString(), collidingDir.ToString())))
		{
			return false;
		}

		LogError(std::format("Entity:{} Has collision with:{} on dira:{} ({})", boxA.GetEntitySafe().GetName(), 
			boxB.GetEntitySafe().GetName(), collidingDir.ToString(), ToString(maybeDirType.value())));
		return m_collisionRegistry.TryAddCollision(CollisionPair(boxA, boxB, intersection, maybeDirType.value()));
	}


	const ColliderOutlineBuffer& CollisionBoxSystem::GetColliderBuffer() const
	{
		return m_colliderOutlineBuffer;
	}

	ColliderOutlineBuffer& CollisionBoxSystem::GetColliderBufferMutable()
	{
		return m_colliderOutlineBuffer;
	}
}
