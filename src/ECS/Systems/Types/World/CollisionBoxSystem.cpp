#include "pch.hpp"
#include "ECS/Systems/Types/World/CollisionBoxSystem.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine::Physics
{
	//static constexpr bool RENDER_COLLIDER_OUTLINES = true;

	CollisionBoxSystem::CollisionBoxSystem(CollisionRegistry& registry) : m_collisionRegistry(registry) {}

	void CollisionBoxSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime)
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
		std::vector<CollisionBoxComponent*> boxes = {};
		scene.GetComponentsMutable<CollisionBoxComponent>(ECS::ALL_ACTIVE_ENABLED_FLAG, boxes);
		if (boxes.empty()) 
			return;

		for (auto& box : boxes)
		{
			box->UpdateCollisionStates();
		}

		for (auto& boxA : boxes)
		{
			if (boxA == nullptr) 
				continue;

			collision = {};
			minBodyDisplacement = {};
			minBodyDisplacementVec = {};

			bounds.emplace_back(std::format("[ENTITY:{} BOX:{}]", boxA->GetEntity().m_Name, boxA->GetAABB().ToString(boxA->GetAABBCenterWorldPos())));

			for (auto& boxB : boxes)
			{
				if (boxB == nullptr || boxA==boxB) 
					continue;

				//Note: we check to make sure we do not have existing collision so we do not consider the same collision
						//twice and accidentally change state
				bool hasThisCollision = m_collisionRegistry.HasCollision(*boxA, *boxB);
				if (hasThisCollision) 
					return;

				//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
				//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
				collision = boxA->GetCollisionIntersectionData(*boxB);

				/*LogError(std::format("Found collisions: between{}({}) and {}({}) ->{}", boxA.ToString(), entityA.GetName(),
					boxB.ToString(), entityB.GetName(), collision.ToString()));*/
					//Assert(false, std::format("Checking collision"));
				if (collision.m_DoIntersect)
				{
					if (std::abs(collision.m_Depth.m_X) <= CollisionBoxComponent::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_X = 0;
					if (std::abs(collision.m_Depth.m_Y) <= CollisionBoxComponent::MAX_DISTANCE_FOR_COLLISION) collision.m_Depth.m_Y = 0;

					if (!Assert(TryAddCollisionToRegistry(*boxA, *boxB, collision), "Tried to add collision:{} to "
						"registry but something went wrong", collision.ToString()))
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

		/*LogError(std::format("All colliders:{} ------ COLLISIONS:{} REGISTRY:{}", ::Utils::ToStringIterable<std::vector<std::string>, std::string>(bounds),
			std::to_string(collisionsAdded), m_collisionRegistry.ToStringCollidingBodies()));*/
	}

	bool CollisionBoxSystem::TryAddCollisionToRegistry(CollisionBoxComponent& boxA, CollisionBoxComponent& boxB, const AABBIntersectionData& intersection)
	{
		const Vec2 collidingDir = intersection.IsTouchingIntersection()? boxB.GetAABBDirection(boxA) : intersection.m_Depth.GetNormalized();
		

		if (intersection.IsTouchingIntersection())
		{
			//Assert(false, std::format("Found touching dir:{}", collidingDir.ToString()));
		}
		std::optional<MoveDirection> maybeDirType = TryConvertVectorToDirection(collidingDir);

		if (!Assert(maybeDirType.has_value(), "Tried to add collision between boxA:{} (entity:{}) and boxB:{} (entity:{}) "
			"but failed to get convert colliding dir:{} to direction type",
			boxA.ToString(), boxA.GetEntity().ToString(), boxB.ToString(), boxB.GetEntity().ToString(), collidingDir.ToString()))
		{
			return false;
		}

		/*LogError(std::format("Entity:{} Has collision with:{} on dira:{} ({})", boxA.GetEntitySafe().GetName(), 
			boxB.GetEntitySafe().GetName(), collidingDir.ToString(), ToString(maybeDirType.value())));*/
		return m_collisionRegistry.TryAddCollision(CollisionPair(boxA, boxB, intersection, maybeDirType.value()));
	}


	const Rendering::ColliderOutlineBuffer& CollisionBoxSystem::GetColliderBuffer() const
	{
		return m_colliderOutlineBuffer;
	}

	Rendering::ColliderOutlineBuffer& CollisionBoxSystem::GetColliderBufferMutable()
	{
		return m_colliderOutlineBuffer;
	}

	std::vector<CollisionBoxComponent*> CollisionBoxSystem::FindBodiesContainingPos(Scenes::Scene& scene, const WorldPosition2D& worldPos) const
	{
		std::vector<CollisionBoxComponent*> bodiesFound = {};

		scene.OperateOnActiveComponents<CollisionBoxComponent>(
			[&bodiesFound, &worldPos](CollisionBoxComponent& box) -> void
			{
				if (box.DoIntersect(worldPos))
				{
					bodiesFound.emplace_back(&box);
				}
			});

		return bodiesFound;
	}
}
