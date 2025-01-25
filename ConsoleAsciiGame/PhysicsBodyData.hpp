#pragma once
#include "ComponentData.hpp"
#include "Point2D.hpp"
#include "Vec2.hpp"
#include "AABB.hpp"

class PhysicsBodyData : public ComponentData
{
private:
	//The bounding box of the rigidbody that is used for collisions
	//Note: this is RELATIVE to the transform
	Physics::AABB m_AABB;
	
	Vec2 m_Velocity;
	Vec2 m_Acceleration;
	

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc

private:
	bool ValidateAABB(const Physics::AABB& bounding) const;

public:
	PhysicsBodyData();
	PhysicsBodyData(const Physics::AABB& boundingBox);

	void SetVelocity(const Vec2& vel);
	void SetVelocityXDelta(const float& xDelta);
	void SetVelocityYDelta(const float& yDelta);

	void SetAcceleration(const Vec2& vel);

	const Vec2& GetVelocity() const;
	const Vec2& GetAcceleration() const;

	const Physics::AABB& GetAABB() const;
};

