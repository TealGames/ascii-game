#pragma once
#include <vector>
#include "ComponentData.hpp"
#include "Point2D.hpp"
#include "Vec2.hpp"
#include "AABB.hpp"
#include "WorldPosition.hpp"

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
	
	Vec2 m_velocity;
	Vec2 m_acceleration;

	std::vector<PhysicsBodyData*> m_collidingBodies;

	float m_gravity;

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc
public:
	static constexpr float NO_COLLISION_DISTANCE_THRESHOLD= 0.01;

private:
	bool ValidateAABB(const Physics::AABB& boundingBox) const;
	Physics::AABB CreateAABB(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);

public:
	PhysicsBodyData();
	PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset, const float& gravity);

	void SetVelocity(const Vec2& vel);
	void SetVelocityXDelta(const float& xDelta);
	void SetVelocityYDelta(const float& yDelta);

	void SetAcceleration(const Vec2& vel);

	const Vec2& GetVelocity() const;
	const Vec2& GetAcceleration() const;
	const float& GetGravity() const;

	const Physics::AABB& GetAABB() const;
	const WorldPosition GetAABBTopLeftWorldPos(const WorldPosition& currentPos) const;

	void AddCollidingBody(PhysicsBodyData& collidingBody);
	void RemoveCollidingBody(const std::vector<PhysicsBodyData*>::iterator& removeBodyIterator);
	const bool& IsCollidingWithAnyBody() const;

	std::vector<PhysicsBodyData*>::iterator GetCollidingBodyIterator(const PhysicsBodyData& physicsBody);
	bool IsValidCollidingBodyIterator(const std::vector<PhysicsBodyData*>::iterator& removeBodyIterator) const;
	PhysicsBodyData* TryGetCollidingBody(const PhysicsBodyData& physicsBody);
	bool IsCollidingWithBody(const PhysicsBodyData& physicsBody);
	int GetTotalBodyCollisions();
};

