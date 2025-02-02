#pragma once
#include <vector>
#include <string>
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
	/// <summary>
	/// The max velocity for both the x and y velocities
	/// </summary>
	float m_terminalYVelocity;
	Vec2 m_acceleration;
	float m_gravity;

	std::vector<PhysicsBodyData*> m_collidingBodies;

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc
public:

private:
	bool ValidateAABB(const Physics::AABB& boundingBox) const;
	Physics::AABB CreateAABB(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);

public:
	PhysicsBodyData();
	PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);
	PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset, const float& gravity, const float& terminalYVelocity);

	void SetVelocity(const Vec2& vel);
	void SetVelocityDelta(const Vec2& vel);
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

	std::string ToStringCollidingBodies() const;
};

