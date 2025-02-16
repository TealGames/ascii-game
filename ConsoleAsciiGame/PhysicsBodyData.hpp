#pragma once
#include <vector>
#include <string>
#include "ComponentData.hpp"
#include "Point2D.hpp"
#include "Vec2.hpp"
#include "AABB.hpp"
#include "WorldPosition.hpp"
#include "Direction.hpp"
#include "NormalizedPosition.hpp"
#include "PhysicsProfile.hpp"

class PhysicsBodyData;
struct CollidingObject
{
	PhysicsBodyData* m_Body = nullptr;
	Direction m_Direction = Direction::Up;
};

namespace Physics
{
	class PhysicsWorld;
}

using CollidingBodiesCollection = std::vector<CollidingObject>;
class PhysicsBodyData : public ComponentData
{
private:
	//The bounding box of the rigidbody that is used for collisions
	//Note: this is RELATIVE to the transform
	Physics::AABB m_aabb;
	/// <summary>
	/// The offset in world position which the bounding box (AABB) lies from the transform position
	/// </summary>
	WorldPosition m_transformOffset;

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

	Physics::PhysicsProfile m_profile;

	CollidingBodiesCollection m_collidingBodies;

	const Physics::PhysicsWorld* m_physicsSimulation;

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc
public:

private:
	bool ValidateAABB(const Physics::AABB& boundingBox) const;
	Physics::AABB CreateAABB(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);

public:
	PhysicsBodyData();
	PhysicsBodyData(const float& mass, const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);
	PhysicsBodyData(const float& mass, const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset, const float& gravity, const float& terminalYVelocity);

	void SetPhysicsWorld(const Physics::PhysicsWorld& world);
	const Physics::PhysicsWorld& GetPhysicsWorldSafe();

	void SetVelocity(const Vec2& vel);
	void SetVelocityDelta(const Vec2& vel);
	void SetVelocityXDelta(const float& xDelta);
	void SetVelocityYDelta(const float& yDelta);

	void SetAcceleration(const Vec2& vel);

	const float& GetMass() const;
	Vec2 GetMomentum() const;
	bool ConservesMomentum() const;
	const Physics::PhysicsProfile& GetPhysicsProfile() const;

	const Vec2& GetVelocity() const;
	const Vec2& GetAcceleration() const;
	const float& GetGravity() const;
	bool IsExperiencingGravity() const;

	const Physics::AABB& GetAABB() const;
	const WorldPosition GetAABBCenterWorldPos() const;
	const WorldPosition GetAABBTopLeftWorldPos() const;
	/// <summary>
	/// Will get the AABB pos based on the relative pos of the AABB
	/// where (0,0) is bottom left and (1, 1) is top right
	/// </summary>
	/// <param name="relativePos"></param>
	/// <returns></returns>
	const WorldPosition GetAABBWorldPos(const NormalizedPosition& relativePos) const;

	void AddCollidingBody(PhysicsBodyData& collidingBody);
	void RemoveCollidingBody(const CollidingBodiesCollection::iterator& removeBodyIterator);
	bool IsCollidingWithBodyInDir(const Direction& dir) const;
	const bool& IsCollidingWithAnyBody() const;

	CollidingBodiesCollection::iterator GetCollidingBodyIterator(const PhysicsBodyData& physicsBody);
	bool IsValidCollidingBodyIterator(const CollidingBodiesCollection::iterator& removeBodyIterator) const;
	PhysicsBodyData* TryGetCollidingBody(const PhysicsBodyData& physicsBody);
	bool IsCollidingWithBody(const PhysicsBodyData& physicsBody);
	int GetTotalBodyCollisions();

	std::string ToStringCollidingBodies() const;
};

