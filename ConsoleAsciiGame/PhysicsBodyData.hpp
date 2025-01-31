#pragma once
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
	Physics::AABB m_AABB;
	/// <summary>
	/// The offset in world position which the bounding box (AABB) lies from the transform position
	/// </summary>
	WorldPosition m_TransformOffset;
	
	Vec2 m_Velocity;
	Vec2 m_Acceleration;
	

	//TODO: add other settings like restitution (bounciness), friction, gravity, etc

private:
	bool ValidateAABB(const Physics::AABB& boundingBox) const;
	Physics::AABB CreateAABB(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);

public:
	PhysicsBodyData();
	PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset);

	void SetVelocity(const Vec2& vel);
	void SetVelocityXDelta(const float& xDelta);
	void SetVelocityYDelta(const float& yDelta);

	void SetAcceleration(const Vec2& vel);

	const Vec2& GetVelocity() const;
	const Vec2& GetAcceleration() const;

	const Physics::AABB& GetAABB() const;
	const WorldPosition GetAABBTopLeftWorldPos(const WorldPosition& currentPos) const;
};

