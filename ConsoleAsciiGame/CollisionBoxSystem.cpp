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
		std::vector<CollisionBoxData*> boxes = {};
		scene.GetComponentsMutable<CollisionBoxData>(boxes);
		if (boxes.empty()) return;

		for (auto& box : boxes)
		{
			box->UpdateCollisionStates();
		}

		for (auto& boxA : boxes)
		{
			if (boxA == nullptr) continue;

			collision = {};
			minBodyDisplacement = {};
			minBodyDisplacementVec = {};

			bounds.emplace_back(std::format("[ENTITY:{} BOX:{}]", boxA->GetEntitySafe().GetName(), boxA->GetAABB().ToString(boxA->GetAABBCenterWorldPos())));
			if (RENDER_COLLIDER_OUTLINES)
			{
				/*if (!Assert(this, mainCamera != nullptr, std::format("Tried to render collider outlines for entity: {} "
					"but the scene:{} has no active camera!", entity->GetName(), scene.GetName()))) return;*/

					//WorldPosition topLeftColliderPos = body->GetAABBTopLeftWorldPos();
				WorldPosition topLeftColliderPos = boxA->GetAABBTopLeftWorldPos();
				//TODO: the camera should convert to screen pos not here
				ScreenPosition topLeftScreenPos = Conversions::WorldToScreenPosition(mainCamera, topLeftColliderPos);
				/*LogWarning(std::format("ADDING OUTLINE for entity: {} pos: {} top left collider: {} SCREEN TOP LEFT: {} half size: {}",
					entity.m_Name, entity.m_Transform.m_Pos.ToString(), topLeftColliderPos.ToString(), topLeftScreenPos.ToString(), body.GetAABB().GetHalfExtent().ToString()));*/

					//m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(body->GetAABB().GetSize(), topLeftScreenPos));
				m_colliderOutlineBuffer.AddRectangle(RectangleOutlineData(boxA->GetAABB().GetSize(), topLeftScreenPos));
			}

			for (auto& boxB : boxes)
			{
				if (boxB == nullptr || boxA==boxB) continue;

				//Note: we check to make sure we do not have existing collision so we do not consider the same collision
						//twice and accidentally change state
				bool hasThisCollision = m_collisionRegistry.HasCollision(*boxA, *boxB);
				if (hasThisCollision) return;

				//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
				//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
				collision = boxA->GetCollisionIntersectionData(*boxB);

				/*LogError(std::format("Found collisions: between{}({}) and {}({}) ->{}", boxA.ToString(), entityA.GetName(),
					boxB.ToString(), entityB.GetName(), collision.ToString()));*/
					//Assert(false, std::format("Checking collision"));
				if (collision.m_DoIntersect)
				{
					if (std::abs(collision.m_Depth.m_X) <= CollisionBoxData::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_X = 0;
					if (std::abs(collision.m_Depth.m_Y) <= CollisionBoxData::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_Y = 0;

					if (!Assert(this, TryAddCollisionToRegistry(*boxA, *boxB, collision), std::format("Tried to add collision:{} to "
						"registry but something went wrong", collision.ToString())))
						return;

					//Note: internal implementation checks if they already have those collisions listed
					boxA->TryAddCollidingBox(*boxB);
					boxB->TryAddCollidingBox(*boxA);

					//LogError(std::format("Box a has new box b flag:{}", ));
				}
				else
				{
					//If we do not have a collision, we try to remove those bodies in case they already were 
					//stored before
					boxA->TryRemoveCollidingBox(*boxB);
					boxB->TryRemoveCollidingBox(*boxA);
				}
			}
		}

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

		/*LogError(std::format("Entity:{} Has collision with:{} on dira:{} ({})", boxA.GetEntitySafe().GetName(), 
			boxB.GetEntitySafe().GetName(), collidingDir.ToString(), ToString(maybeDirType.value())));*/
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

	std::vector<CollisionBoxData*> CollisionBoxSystem::FindBodiesContainingPos(Scene& scene, const WorldPosition& worldPos) const
	{
		std::vector<CollisionBoxData*> bodiesFound = {};

		scene.OperateOnComponents<CollisionBoxData>(
			[&bodiesFound, &worldPos](CollisionBoxData& box, ECS::Entity& entity) -> void
			{
				if (box.DoIntersect(worldPos))
				{
					bodiesFound.emplace_back(&box);
				}
			});

		return bodiesFound;
	}
}
