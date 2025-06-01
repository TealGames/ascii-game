#include "pch.hpp"
#include "PhysicsWorld.hpp"
#include "EntityData.hpp"
#include "ProfilerTimer.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "CollisionBoxData.hpp"
#include "CollisionRegistry.hpp"

namespace Physics
{
	bool HasFlagEntityA(const EntityType& entityType)
	{
		const std::uint8_t typeConverted = static_cast<std::uint8_t>(entityType);
		return typeConverted == static_cast<std::uint8_t>(EntityType::A) ||
			   typeConverted == static_cast<std::uint8_t>(EntityType::AB);
	}
	bool HasFlagEntityB(const EntityType& entityType)
	{
		const std::uint8_t typeConverted = static_cast<std::uint8_t>(entityType);
		return typeConverted == static_cast<std::uint8_t>(EntityType::B) ||
			typeConverted == static_cast<std::uint8_t>(EntityType::AB);
	}

	PhysicsWorld::PhysicsWorld(CollisionRegistry& collisionRegistry)
		: m_collisionRegistry(collisionRegistry), m_bodies() {}


	const PhysicsBodyCollection& PhysicsWorld::GetBodies() const
	{
		return m_bodies;
	}

	PhysicsBodyCollection& PhysicsWorld::GetBodiesMutable()
	{
		return m_bodies;
	}

	void PhysicsWorld::AddBody(PhysicsBodyData& body)
	{
		//(this, std::format("Adding body: {}", body.m_Entity->GetName()));
		body.SetPhysicsWorldRef(*this);
		m_bodies.push_back(&body);
	}

	void PhysicsWorld::ClearAllBodies()
	{
		for (auto& body : m_bodies)
		{
			if (body == nullptr) continue;
			body->RemovePhysicsWorldRef();
		}
		m_bodies = {};
	}

	void PhysicsWorld::UpdateStart(const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("PhysicsWorld::Update");
#endif 

		m_nonGroundedCollidingEntities.clear();
		m_nonGroundedCollidingEntities.reserve(m_collisionRegistry.GetTotalCollisionsCount());

		m_collisionRegistry.ExecuteOnAllCollisions([this](CollisionPair& collisionData) -> void
			{
				if (!Assert(this, collisionData.m_CollisionBoxA != nullptr, std::format("Tried to resolve a collision:{} "
					"but collision box A is null", collisionData.ToString())))
					return;

				if (!Assert(this, collisionData.m_CollisionBoxB != nullptr, std::format("Tried to resolve a collision:{} "
					"but collision box B is null", collisionData.ToString())))
					return;

				//TODO: if one has physics body while other does not do we still simulate physics?
				PhysicsBodyData* bodyA = collisionData.m_CollisionBoxA->GetEntitySafeMutable().TryGetComponentMutable<PhysicsBodyData>();
				if (bodyA == nullptr) return;

				PhysicsBodyData* bodyB = collisionData.m_CollisionBoxB->GetEntitySafeMutable().TryGetComponentMutable<PhysicsBodyData>();
				if (bodyB == nullptr) return;

				//Assert(false, std::format("Found collision"));
				//if (collisionData.m_CollisionBoxA->GetEntitySafe().GetName() == "player" ||
				//	collisionData.m_CollisionBoxB->GetEntitySafe().GetName() == "player")
				//{
				//	LogError(std::format("Has collision touching: {} data:{}", std::to_string(collisionData.m_IntersectionData.IsTouchingIntersection()), collisionData.ToString()));
				//	//if (!collisionData.m_IntersectionData.IsTouchingIntersection()) Assert(false, "NON TCOUHGIN COLLISON");
				//}
				
				//Since touching collisions are not actually interecting inside another body we should not
				//apply any methods of collision resolution (but we still need to know they are colliding 
				//for things like grounded checks/gravity)
				
				//TODO: using global pos everywere is expensive, perhaps we can optimize by checking parents first?
				const Vec2 incomingBToADir = GetVector(bodyB->GetEntitySafe().GetTransform().GetGlobalPos(), bodyA->GetEntitySafe().GetTransform().GetGlobalPos()).GetYAsVector();
				const float dotProductBofA = DotProduct(incomingBToADir, bodyB->GetVelocity());
				/*LogError(std::format("touching:{} DOT BETWEEN B-> A:{} bodyB vel:{} is:{}", std::to_string(collisionData.m_IntersectionData.IsTouchingIntersection()), incomingBToADir.ToString(),
					bodyB->GetVelocity().ToString(), std::to_string(dotProductBofA)));*/
				if (collisionData.m_IntersectionData.IsTouchingIntersection() && dotProductBofA <=0)
				{
					//Assert(false, std::format("YES"));
					return;
				}


				ResolveCollision(collisionData, bodyA, bodyB);

				/*
				//Note: we need to remove the collision body from registry since if we cycel throguh all entities
				//those that have multiple collisions may get resolved by their parter body first thus leading to
				//an attempted resolution twice for the same pair
				if (!Assert(this, m_collisionRegistry.TryRemoveCollision(collisionData), std::format("Tried to remove a collision pair "
					"from registry in collision resolution but faield for pair:{}", collisionData.ToString())))
					return;
				*/
			});
		//LogError(std::format("All collisions after update:{}", m_collisionRegistry.ToStringCollidingBodies()), true, false, false, true);

		//AABBIntersectionData collision = {};
		//Vec2 minBodyDisplacement = {};
		//Vec2 minBodyDisplacementVec = {};
		for (int i = 0; i < m_bodies.size(); i++)
		{
			if (m_bodies[i] == nullptr) continue;

			PhysicsBodyData& body = *(m_bodies[i]);
			const CollisionBoxData& box = m_bodies[i]->GetCollisionBox();
			EntityData& bodyAEntity = body.GetEntitySafeMutable();

			KinematicUpdate(deltaTime, bodyAEntity, body, box);

			//We do not need to simulate bodies with no movement since collisions are resolved on bodies that move
			//if (Utils::ApproximateEqualsF(bodyA.GetVelocity().GetMagnitude(), 0)) continue;
			//collision = {};

			/*
			//Note: this could get optimized to not need double checking for each possible collision
			//but we need it in this time to ensure that velocity and other updates occur for BOTH bodies
			//and not just to one
			for (int j = 0; j < m_bodies.size(); j++)
			{
				if (j == i) continue;
				if (m_bodies[j] == nullptr) continue;

				PhysicsBodyData& bodyB = *(m_bodies[j]);
				ECS::Entity& bodyBEntity = bodyB.GetEntitySafeMutable();

				auto entityACollisionBIt = bodyA.GetCollidingBodyIterator(bodyB);
				bool isValidIt = bodyA.IsValidCollidingBodyIterator(entityACollisionBIt);

				//Intersection is handled as BODYA is the body that is colliding with BODYB (Pretending as though bodyb is not moving)
				//NOTE: so we are essentially saying is BODY A encroaching on any other bodies space and if so do something
				//collision = Physics::GetAABBIntersectionData(bodyBEntity.m_Transform.m_Pos, bodyB.GetAABB(), bodyAEntity.m_Transform.m_Pos, bodyA.GetAABB());
				collision = bodyB.GetCollisionBox().GetCollisionIntersectionData(bodyA.GetCollisionBox());
				if (collision.m_DoIntersect)
				{
					//IF BODYA has moved this past frame (it means it must have moved into bodyB)
					if (bodyA.GetEntitySafe().m_Transform.m_Pos!= bodyA.GetEntitySafe().m_Transform.m_LastPos)
					{
						if (bodyA.GetVelocity() == Vec2::ZERO || !bodyA.ConservesMomentum() || !bodyB.ConservesMomentum())
						{
							bool xIsMin = std::abs(collision.m_Depth.m_X) < std::abs(collision.m_Depth.m_Y);
							float moveDelta = -(xIsMin ? collision.m_Depth.m_X : collision.m_Depth.m_Y);
							//We add the extra distance so that they are still considered as colliding but would not cause any awkward jittering
							//moveDelta += Utils::GetSign(moveDelta) * MAX_DISTANCE_FOR_COLLISION;

							if (xIsMin) bodyAEntity.m_Transform.SetPosDeltaX(moveDelta);
							else bodyAEntity.m_Transform.SetPosDeltaY(moveDelta);

							bodyA.SetAcceleration(Vec2::ZERO);
							bodyA.SetVelocity(Vec2::ZERO);
						}
						else
						{
							//TODO: the collision normal does not fully consider cases where things may hit at angles
							const Vec2 collsionNormal = bodyA.GetVelocity().GetOppositeDirection().GetNormalized();
							const Vec2& oldMomenumn = bodyA.GetMomentum();
							const Vec2 newMomentum = oldMomenumn + (collsionNormal * CalculateImpulse(bodyA, bodyB, collsionNormal));



							Vec2 newVel = newMomentum / bodyA.GetMass();
							if (std::abs(newVel.m_X) <= BOUNCE_END_SPEED_THRESHOLD) newVel.m_X = 0;
							if (std::abs(newVel.m_Y) <= BOUNCE_END_SPEED_THRESHOLD) newVel.m_Y = 0;


							Vec2 moveDelta = collision.m_Depth.GetOppositeDirection();
							bodyAEntity.m_Transform.SetPosDelta({ moveDelta.m_X, moveDelta.m_Y });

							Log(this, std::format("COSNERVING for {} MASS A: {} MASS B: {} DEPTH: {} in norma: {} old p: {} j delta;{} new: {} vel: {}",
								bodyA.GetEntitySafe().GetName(), std::to_string(bodyA.GetMass()), std::to_string(bodyB.GetMass()),
								moveDelta.ToString(),
								collsionNormal.ToString(), oldMomenumn.ToString(), std::to_string(CalculateImpulse(bodyA, bodyB, collsionNormal)),
								newMomentum.ToString(), (newMomentum / bodyA.GetMass()).ToString()));

							bodyA.SetAcceleration(Vec2::ZERO);
							bodyA.SetVelocity(newVel);
							//throw std::invalid_argument("FART");
						}

						//TODO: problem is if there is body above and below, making it possible for it to move back and worth between collisions of the two
						//minBodyDisplacement = GetBodyMinDisplacement(bodyB, bodyA);
						//minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
						//LogWarning(std::format("After collision prevented new distance: {}", minBodyDisplacementVec.ToString()));
						//throw new std::invalid_argument("yeah");
					}
					if (!isValidIt) bodyA.AddCollidingBody(bodyB);
					break;
				}


				minBodyDisplacement = GetBodyMinDisplacement(bodyB, bodyA);
				minBodyDisplacementVec = { std::abs(minBodyDisplacement.m_X), std::abs(minBodyDisplacement.m_Y) };
				//If we pass intersect check (meaning we didnt intersect) and it was valid we can remove iterator if we pass
				//the threshold in order to allow for some degree of touching to still be considered as colliding
				if (isValidIt && minBodyDisplacementVec.GetMagnitude() > MAX_DISTANCE_FOR_COLLISION)
				{
					bodyA.RemoveCollidingBody(entityACollisionBIt);
				}

			}
			*/

			/*LogWarning(std::format("COLLISION FOR ENTITY: {} FOUND: {} DEPTH: {}", bodyA.m_Entity->m_Name,
				std::to_string(collision.m_DoIntersect), collision.m_Depth.ToString()));*/
		}
	}

	void PhysicsWorld::ResolveCollision(CollisionPair& collision, PhysicsBodyData* bodyA, PhysicsBodyData* bodyB)
	{
		EntityData& entityA = collision.m_CollisionBoxA->GetEntitySafeMutable();
		EntityData& entityB = collision.m_CollisionBoxB->GetEntitySafeMutable();

		if (!Assert(this, bodyA != nullptr && bodyB != nullptr, std::format("Tried to resolve collision:{} body bodyA and/or bodyB "
			"has no physicsBody", collision.ToString())))
			return;

		bool bothBodiesZeroVelocity = bodyA->GetVelocity() == Vec2::ZERO && bodyB->GetVelocity() == Vec2::ZERO;

		//By default, we always want to push one of the bodies out
		PushMovedBodyOut(entityA, entityB, *bodyA, *bodyB, collision);

		//If the movement happened instantly (both have no velocities) then we can exit after push out
		if (bothBodiesZeroVelocity)
		{
			//Assert(false, std::format("Leaving due to no vel"));
			return;
		}

		//If both have mass (and both are NOT constrained) we can apply impulse to resolve collision
		//and we can just not set the constrained object's position
		if (bodyA->HasMass() && bodyB->HasMass())
		{
			//TODO: consider external forces which results in no conservation of momentum
			//Assert(false, std::format("IMPUSLESE"));
			ApplyImpulse(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData);
			/*LogError(std::format("APPLYING IMPULSE for depth:{} separation:{}", collision.m_IntersectionData.ToString(), 
				std::to_string((entityA.m_Transform.GetPos() > entityB.m_Transform.GetPos()? 
					collision.m_CollisionBoxA->GetGlobalMin() - collision.m_CollisionBoxB->GetGlobalMax() :
					collision.m_CollisionBoxA->GetGlobalMax() - collision.m_CollisionBoxB->GetGlobalMin()
				).m_Y)), true, false, false, true);*/

			return;
		}

		const bool isConstrainedA = bodyA->HasAnyConstraints();
		const bool isConstrainedB = bodyB->HasAnyConstraints();
		//If one is constrianed but the other is not, the one that is moving gets the new velocity
		//applied from the restitution setting in the physics profile (mass ignored)
		if (isConstrainedA!= isConstrainedB)
		{
			//Assert(false, std::format("single constraint"));
			EntityType updatedEntity = bodyA->HasAnyConstraints() ? EntityType::B : EntityType::A;
			SetVelocitiesFromRestitution(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData, updatedEntity);
			return;
		}
		//If none of them are constrained (and either one has mass or neither have mass)
		//we can then use similar velocity setting from restitution but on both entities
		if (!isConstrainedA && !isConstrainedB)
		{
			SetVelocitiesFromRestitution(entityA, entityB, *bodyA, *bodyB, collision.m_IntersectionData, EntityType::AB);
			return;
		}

		LogError(this, std::format("Tried to handle collision:{} but no previous resolution methods "
			"were possible so simple push is executed", collision.ToString()));
		return;
	}

	void PhysicsWorld::KinematicUpdate(const float& deltaTime, EntityData& entity, PhysicsBodyData& body, const CollisionBoxData& box)
	{
		body.SetIsGrounded(m_collisionRegistry.IsCollidingInDirs(box,
			{ MoveDirection::South, MoveDirection::Southeast, MoveDirection::Southwest }, true));
		//if (isGrounded) Assert(false, std::format("BALLS"));

		/*if (entity.GetName()=="player") LogError(std::format("Player has down collisions:{} total collisions:{}", 
			std::to_string(body.IsGrounded()), Utils::ToStringIterable<std::vector<MoveDirection>, MoveDirection>(m_collisionRegistry.TryGetCollisionDirs(box))));*/

		bool gravitySet = false;
		if (!body.IsGrounded() && !body.IsExperiencingGravity())
		{
			body.SetAcceleration({ body.GetAcceleration().m_X, body.GetGravity() });
		}
		else if (body.IsGrounded() && body.IsExperiencingGravity())
		{
			gravitySet = true;
			body.SetAcceleration({body.GetAcceleration().m_X, 0});
			//body.SetVelocity(Vec2::ZERO);
			//Assert(false, std::format("gravity removed"));
			//return;
		}

		body.SetVelocityDelta(body.GetAcceleration() * deltaTime);

		float moveX = body.GetVelocity().m_X * deltaTime;
		if (body.HasXConstraint()) moveX = 0;

		float moveY = body.GetVelocity().m_Y * deltaTime;
		if (body.HasYConstraint()) moveY = 0;

		//LogWarning(std::format("ENTITY SETTING POS: {}", std::to_string(xVelocity), std::to_string(yVelocity)));
		entity.GetTransformMutable().SetLocalPosDelta(Vec2(moveX, moveY));

		//if (gravitySet) LogError(std::format("graivyt set for:{} new a:{} new v:{}", entity.GetName(), body.GetAcceleration().ToString(), body.GetVelocity().ToString()), true, false, false, true);

		/*
			LogError(std::format("Setting acceleration! Collisions: {} vel: {} vvel magnitude: {} ENTITY: {} has COLLIDING BODIES: {}",
				std::to_string(bodyA.GetTotalBodyCollisions()), bodyA.GetVelocity().ToString(),
				std::to_string(bodyA.GetVelocity().GetMagnitude()), bodyA.GetEntitySafe().m_Name, bodyA.ToStringCollidingBodies()));
			*/
	}

	void PhysicsWorld::PushMovedBodyOut(EntityData& entityA, EntityData& entityB,
		PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const CollisionPair& collision)
	{
		//If A has constraints or both have constraints (since it is first choice A will get choosen)
		//then we select A, otherwise we select B if A has constrinats. If none have constraints,
		//we simply pick the one that moved last frame

		bool isMoveEntityB = false;
		if (bodyB.HasAnyConstraints())
		{
			isMoveEntityB = false;
			if (bodyA.HasAnyConstraints())
			{
				LogError(this, std::format("Handling collision for pair:{} but both are constrained "
					"objects so entityA is pushed out", collision.ToString()));
			}
		}
		else if (bodyA.HasAnyConstraints()) isMoveEntityB = true;
		else isMoveEntityB = false;

		EntityData* movedEntity = isMoveEntityB ? &entityB : &entityA;

		//Note: the pentration depth will be the same for both bodies (they might just have different signs)
		const bool xIsMin = std::abs(collision.m_IntersectionData.m_Depth.m_X) < std::abs(collision.m_IntersectionData.m_Depth.m_Y) 
			&& collision.m_IntersectionData.m_Depth.m_X != 0;

		//Note: since depth is for B penetration into A, if it was B that moved we need reverse depth to get out
		//otherwise if it is A, the depth would be the reverse of depth data, thus we do not need to change direction of move
		const int deltaSign = movedEntity == &entityA ? 1 : -1;
		const float moveDelta = (xIsMin ? collision.m_IntersectionData.m_Depth.m_X : collision.m_IntersectionData.m_Depth.m_Y) * deltaSign;
		/*if (entityA.GetName()=="player" || entityB.GetName() == "player") 
			LogError(std::format("Move delta:{} for collision:{} BMOVED:{} entityB:{}", std::to_string(moveDelta), 
				collision.ToString(), std::to_string(isMoveEntityB), entityB.GetName()));*/

		if (xIsMin) movedEntity->GetTransformMutable().SetLocalPosDeltaX(moveDelta);
		else movedEntity->GetTransformMutable().SetLocalPosDeltaY(moveDelta);

		//Whichever one got moved out should not have any further movement to prevent potential jittering
		//PhysicsBodyData& movedBody = aMovedLastFrame ? bodyA : bodyB;
		//movedBody.SetAcceleration(Vec2::ZERO);
		//bodyA.SetAcceleration(Vec2::ZERO);
		//bodyB.SetAcceleration(Vec2::ZERO);
	}

	void PhysicsWorld::ApplyImpulse(EntityData& entityA, EntityData& entityB,
		PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const AABBIntersectionData& intersectionData)
	{
		//Assert(false, std::format("Intersection is:{}", collision.m_IntersectionData.ToString()));
			//const Vec2 collsionNormal = bodyA.GetVelocity().GetOppositeDirection().GetNormalized();
			//const Vec2 collsionNormal = GetCollisionNormal(*collision.m_CollisionBoxA, *collision.m_CollisionBoxB);
		//Assert(false, std::format("collision normal:{}", collsionNormal.ToString()));

		const float e = (bodyA.GetPhysicsProfile().GetRestitution() +
			bodyB.GetPhysicsProfile().GetRestitution()) / 2;

		const Vec2 collsionNormalA = GetCollisionNormalBodyB(intersectionData).GetOppositeDirection();

		//If the collision normal is 0, it means no meaningful collision happened
		//if (collsionNormalA.m_X == 0 && collsionNormalA.m_Y == 0) return;

		const float mA = bodyA.GetMass();
		const float mB = bodyB.GetMass();
		const Vec2 vnOA = collsionNormalA * DotProduct(bodyA.GetVelocity(), collsionNormalA);
		const Vec2 vnOB = collsionNormalA * DotProduct(bodyB.GetVelocity(), collsionNormalA);

		const Vec2 vNormalFinalA = ((vnOA * mA) + (vnOB * mB) - ((vnOA - vnOB) * e * mB)) / (mA + mB);
		Vec2 newVelA = vNormalFinalA + (bodyA.GetVelocity() - vnOA);
		//const Vec2& oldMomenumnA = bodyA.GetMomentum();

		//const Vec2 velNormalA = collsionNormalA * bodyA.GetVelocity();
		/*Vec2 newVelA = (bodyA.GetVelocity() * (bodyA.GetMass() - bodyB.GetMass() * averageE) + bodyB.GetMomentum() * (1 + averageE))
			/ (bodyA.GetMass() + bodyB.GetMass()) + (bodyA.GetVelocity() - velNormalA);*/

		const Vec2 impulseA = (newVelA * bodyA.GetMass()) - bodyA.GetMomentum();


		//const Vec2 impulseA = collsionNormal * CalculateImpulse(bodyA, bodyB, collsionNormal);


		//Since impulse gained by one is the one lost by other, we calculate impulse A then apply opposite
		//to be to find both of their new velocities because of momentum conservation
		//Vec2 newVelA = (oldMomenumnA + impulseA) / bodyA.GetMass();
		
		/*Assert(false, std::format("collision:{} mA:{} mB:{} oldVelA:{} oldVelB:{} bodyA:{} nprmal:{} old momentumA:{} impulseA:{} newVelA:{} entityB:{} new velB:{}",
			intersectionData.ToString(), std::to_string(mA), std::to_string(mB), bodyA.GetVelocity().ToString(), bodyB.GetVelocity().ToString(),
			entityA.GetName(), collsionNormalA.ToString(), bodyA.GetMomentum().ToString(), impulseA.ToString(),
			newVelA.ToString(), entityB.GetName(), newVelB.ToString()));*/

		//We clamp velocities if they surpass bounce speed threshold so they do not bounce forever 
		//with very small bounce heights/jittery look
		

		//We move entityA outside of colliding body (note: in reality it does not matter which one we move out)
		//Vec2 moveDelta = intersectionData.m_Depth.GetOppositeDirection();
		//entityA.m_Transform.SetPosDelta({ moveDelta.m_X, moveDelta.m_Y });

		if (!bodyA.HasAnyConstraints())
		{
			//We apply a threshold to stop velocity when the velocity is small enough to prevent jittering
			if (std::abs(newVelA.GetMagnitude()) <= BOUNCE_END_SPEED_THRESHOLD)
			{
				newVelA = {};
				//Assert(false, std::format("SETTING 0 VEL IN IMPUSELSE NOW IT IS:{}", newVelA.ToString()));
			}

			//bodyA.SetAcceleration(Vec2::ZERO);
			bodyA.SetVelocity(newVelA);
		}
		
		if (!bodyB.HasAnyConstraints())
		{
			Vec2 newVelB = (bodyB.GetMomentum() - impulseA) / bodyB.GetMass();
			//We apply a threshold to stop velocity when the velocity is small enough to prevent jittering
			if (std::abs(newVelB.GetMagnitude()) <= BOUNCE_END_SPEED_THRESHOLD)
			{
				newVelB = {};
				//Assert(false, std::format("SETTING 0 VEL IN IMPUSELSE"));
			}

			//bodyB.SetAcceleration(Vec2::ZERO);
			bodyB.SetVelocity(newVelB);
		}
		//throw std::invalid_argument("FART");
		/*Assert(false, std::format("Setting bodfyA vel:{}(shouldbe:{}) bodyB vel:{}(shouldbe:{})", bodyA.GetVelocity().ToString(), 
			newVelA.ToString(), bodyB.GetVelocity().ToString(), newVelB.ToString()));*/
	}

	//PRECONDITION: we assume that one of the bodies is NOT constrained
	void PhysicsWorld::SetVelocitiesFromRestitution(EntityData& entityA, EntityData& entityB,
		PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const AABBIntersectionData& intersectionData, 
		const EntityType updateEntityType)
	{
		const Vec2 collsionNormalA = GetCollisionNormalBodyB(intersectionData).GetOppositeDirection();

		const float averageRestitution = (bodyA.GetPhysicsProfile().GetRestitution() +
			bodyB.GetPhysicsProfile().GetRestitution()) / 2;

		Vec2 newBodyNormal = {};
		Vec2 velocityNormal = {};

		// For each entity if we have to update it, we first get normal component of old velocity 
		//Then we update it based on restitution (how much velocity is left) in opposite direction
		//The new velocity is the vector in normal (collision) dir + tangential velocity unchanged (old vel - old vel normal)
		if (HasFlagEntityA(updateEntityType))
		{
			velocityNormal = collsionNormalA * DotProduct(bodyA.GetVelocity(), collsionNormalA);
			newBodyNormal= velocityNormal * -(averageRestitution);
			bodyA.SetVelocity(newBodyNormal + (bodyA.GetVelocity() - velocityNormal));
		}
		if (HasFlagEntityB(updateEntityType))
		{
			//Since collision normal is in terms of A, B's normal is the opposite direction
			velocityNormal = collsionNormalA.GetOppositeDirection() * DotProduct(bodyB.GetVelocity(), collsionNormalA.GetOppositeDirection());
			newBodyNormal = velocityNormal * -(averageRestitution);
			bodyB.SetVelocity(newBodyNormal + (bodyB.GetVelocity() - velocityNormal));
		}
	}

	/*void PhysicsWorld::GetNewCollisionVe(ECS::Entity& entityA, ECS::Entity& entityB,
		PhysicsBodyData& bodyA, PhysicsBodyData& bodyB, const AABBIntersectionData& intersectionData)*/

	Vec2 PhysicsWorld::GetCollisionNormalBodyB(const AABBIntersectionData& data)
	{
		//return (boxB.GetAABBCenterWorldPos() - boxA.GetAABBCenterWorldPos()).GetNormalized();
		return data.m_Depth.GetNormalized();
	}

	float PhysicsWorld::CalculateImpulse(const PhysicsBodyData& targetObject, const PhysicsBodyData& collidedObject, const Vec2& collisionNormal)
	{
		const float averageRestitution = (targetObject.GetPhysicsProfile().GetRestitution() +
									collidedObject.GetPhysicsProfile().GetRestitution()) / 2;

		const float& massA = targetObject.GetMass();
		const float& massB = collidedObject.GetMass();

		const Vec2& velA = targetObject.GetVelocity();
		const Vec2& velB = collidedObject.GetVelocity();
		//LogError(std::format("Dot product for{} dot {} is: {}", velA.ToString(), velB.ToString(),std::to_string(DotProduct(velA, velB)) ));
		return ((1 + averageRestitution) * massA * massB) / (massA + massB) * 
			   (DotProduct(velB, collisionNormal) - DotProduct(velA, collisionNormal));
	}

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		/*return DoAABBIntersect(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(), 
							   body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());*/
		return body1.GetCollisionBox().DoIntersect(body2.GetCollisionBox());
	}

	Vec2 GetBodyMinDisplacement(const PhysicsBodyData& body1, const PhysicsBodyData& body2)
	{
		/*return GetAABBMinDisplacement(body1.m_Entity->m_Transform.m_Pos, body1.GetAABB(),
			body2.m_Entity->m_Transform.m_Pos, body2.GetAABB());*/
		return body1.GetCollisionBox().GetAABBMinDisplacement(body2.GetCollisionBox());
	}

	RaycastInfo PhysicsWorld::Raycast(const WorldPosition& origin, const Vec2& ray) const
	{
		RaycastInfo result = {};
		int xSign = Utils::GetSign(ray.m_X);
		int ySign = Utils::GetSign(ray.m_Y);
		WorldPosition rayEndPoint = GetVectorEndPoint(origin, ray);
		Vec2 rayDir = ray.GetNormalized();

		WorldPosition boundsWorldMin = {};
		WorldPosition boundsWorldMax = {};
		
		float t1 = 0;
		float t2 = 0;

		//Since we consider t values from origin and onward (where tMin represents origin point and tMax represents ray end)
		float tMin = 0;
		float tMax = std::numeric_limits<float>::infinity();
		
		for (auto& body : m_bodies)
		{
			boundsWorldMin = body->GetCollisionBox().GetGlobalMin();
			boundsWorldMax= body->GetCollisionBox().GetGlobalMax();

			//TODO: perhaps optimizations could be made by checking to see if distance is too big to make it to this collider
			//so we can just continue

			//----------- X AXIS -----------
			//If the ray is vertical and outside box, there are no intersections here
			if (Utils::ApproximateEqualsF(ray.m_X, 0) &&
				(origin.m_X < boundsWorldMin.m_X || origin.m_X > boundsWorldMax.m_X))
			{
				/*LogError(std::format("PERFECT LINE origin: {} ray; {} body: {} (body pos: {}) body min: {} max: {}", 
					origin.ToString(), ray.ToString(), body->m_Entity->m_Name, body->GetEntitySafe().m_Transform.m_Pos.ToString(), 
					boundsWorldMin.ToString(), boundsWorldMax.ToString()));*/
				continue;
			}

			t1 = (boundsWorldMin.m_X - origin.m_X) / rayDir.m_X;
			t2= (boundsWorldMax.m_X - origin.m_X) / rayDir.m_X;
			//Swap in case the direction is negative meaning the closer and far vertical bounds are reversed
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				//LogError(std::format("SKIPPING DUE TO MIN: {} > max; {}", std::to_string(tMin), std::to_string(tMax)));
				continue;
			}
			
			//----------- Y AXIS -----------
			//Repeats the same process but for y axis
			if (Utils::ApproximateEqualsF(ray.m_Y, 0) &&
				(origin.m_Y < boundsWorldMin.m_Y || origin.m_Y > boundsWorldMax.m_Y))
			{
				continue;
			}

			t1 = (boundsWorldMin.m_Y - origin.m_Y) / rayDir.m_Y;
			t2 = (boundsWorldMax.m_Y - origin.m_Y) / rayDir.m_Y;
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) continue;

			
			//Note: when reaching this point tMin is the intersecction distance from origin
			result.m_BodyHit = body;
			result.m_Displacement = {rayDir.m_X * tMin, rayDir.m_Y * tMin};
			result.m_HitPos = origin + Vec2(result.m_Displacement.m_X, result.m_Displacement.m_Y);

			/*LogError(std::format("REACHED LINE END origin {} ray; {} body: {} (body pos: {}) body min: {} max: {} displacement; {}",
				origin.ToString(), ray.ToString(), body->m_Entity->m_Name, body->GetEntitySafe().m_Transform.m_Pos.ToString(),
				boundsWorldMin.ToString(), boundsWorldMax.ToString(), result.m_Displacement.ToString()));*/
			return result;
		}
		return result;
	}
}


