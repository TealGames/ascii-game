#pragma once
#include <vector>
#include <string>
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Vec2.hpp"
//#include "Utils/Data/WorldPosition.hpp"
//#include "Utils/Data/Direction.hpp"
//#include "Utils/Data/NormalizedPosition.hpp"
#include "Core/Physics/PhysicsProfile.hpp"
#include "ECS/Component/Types/World/CollisionBoxData.hpp"

namespace Physics
{
	class PhysicsWorld;
}

struct MoveContraints
{
	/// <summary>
	/// If TRUE, means the object cannot move in the x direction (left right)
	/// </summary>
	bool m_ConstrainX;
	/// <summary>
	/// If TRUE, the object cannot move in the Y direction (up down)
	/// </summary>
	bool m_ConstrainY;

	MoveContraints(const bool constrainX = false, const bool constrainY = false);
};

//class CollisionBoxData;
namespace ECS { class PhysicsBodySystem; }
class PhysicsBodyData : public Component
{
private:
	//The bounding box of the rigidbody that is used for collisions
	//Note: this is RELATIVE to the transform
	//Physics::AABB m_aabb;
	const CollisionBoxData* m_collider;

	/// <summary>
	/// The offset in world position which the bounding box (AABB) lies from the transform position
	/// </summary>
	//WorldPosition m_transformOffset;

	/// <summary>
	/// The mass of the object in Kilograms. 
	/// Guaranteed to be >=0 where 0 means momentum will not be considered for this object
	/// Note: this is only useful for determining impulse/conservation of momentum
	/// </summary>
	float m_mass;
	
	Vec2 m_velocity;
	/// <summary>
	/// The max velocity for both the x and y velocities
	/// </summary>
	float m_terminalYVelocity;
	Vec2 m_acceleration;

	//TODO: maybe the physics sim should have its own gravity for all objects rather than each having their own?
	float m_gravity;
	/// <summary>
	/// True if on a ground -> NOT experiencing any gravity acceleration 
	/// </summary>
	bool m_isGrounded;

	Physics::PhysicsProfile m_profile;
	/// <summary>
	/// Constraints on the movement of this object. Note: if it is constrained, pos CAN still be changed
	/// via transform, but physics system is limited in what position(s) it can change
	/// </summary>
	MoveContraints m_contraints;

	const Physics::PhysicsWorld* m_physicsSimulation;

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc
public:
	friend class ECS::PhysicsBodySystem;

private:
	//bool ValidateAABB(const Physics::AABB& boundingBox) const;
	//Physics::AABB CreateAABB(const Vec2& boundingBoxSize, const WorldPosition& transformOffset);

public:
	PhysicsBodyData();
	PhysicsBodyData(const Json& json);
	PhysicsBodyData(const CollisionBoxData* collisionBox, const float mass);
	PhysicsBodyData(const CollisionBoxData* collisionBox, const float mass, 
		const float gravity, const float terminalYVelocity);

	void SetPhysicsWorldRef(const Physics::PhysicsWorld& world);
	void RemovePhysicsWorldRef();
	const Physics::PhysicsWorld& GetPhysicsWorldSafe();

	void SetVelocity(const Vec2& vel);
	void SetVelocityDelta(const Vec2& vel);
	void SetVelocityXDelta(const float& xDelta);
	void SetVelocityYDelta(const float& yDelta);

	void SetAcceleration(const Vec2& vel);

	const float& GetMass() const;
	Vec2 GetMomentum() const;
	bool HasMass() const;
	const Physics::PhysicsProfile& GetPhysicsProfile() const;

	const Vec2& GetVelocity() const;
	const Vec2& GetAcceleration() const;
	const float& GetGravity() const;
	bool IsExperiencingGravity() const;

	void SetIsGrounded(const bool grounded);
	bool IsGrounded() const;

	const CollisionBoxData& GetCollisionBox() const;

	void SetConstraint(const MoveContraints& constraint);
	MoveContraints GetConstraint() const;
	bool HasXConstraint() const;
	bool HasYConstraint() const;
	bool HasAnyConstraints() const;

	/*
	const Physics::AABB& GetAABB() const;
	const WorldPosition GetAABBCenterWorldPos() const;
	const WorldPosition GetAABBTopLeftWorldPos() const;
	bool DoesAABBContainPos(const WorldPosition& pos) const;
	/// <summary>
	/// Will get the AABB pos based on the relative pos of the AABB
	/// where (0,0) is bottom left and (1, 1) is top right
	/// </summary>
	/// <param name="relativePos"></param>
	/// <returns></returns>
	const WorldPosition GetAABBWorldPos(const NormalizedPosition& relativePos) const;
	*/

	/*
	void AddCollidingBody(PhysicsBodyData& collidingBody);
	void RemoveCollidingBody(const CollidingBodiesCollection::iterator& removeBodyIterator);
	bool IsCollidingWithBodyInDirs(const std::vector<MoveDirection>& dirs) const;
	const bool& IsCollidingWithAnyBody() const;

	CollidingBodiesCollection::iterator GetCollidingBodyIterator(const PhysicsBodyData& physicsBody);
	bool IsValidCollidingBodyIterator(const CollidingBodiesCollection::iterator& removeBodyIterator) const;
	PhysicsBodyData* TryGetCollidingBody(const PhysicsBodyData& physicsBody);
	bool IsCollidingWithBody(const PhysicsBodyData& physicsBody);
	int GetTotalBodyCollisions();

	std::string ToStringCollidingBodies() const;
	*/

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

