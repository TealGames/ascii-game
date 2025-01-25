#include "pch.hpp"
#include "PhysicsBodyData.hpp"
#include "HelperFunctions.hpp"

PhysicsBodyData::PhysicsBodyData() 
	: ComponentData(), m_AABB(),  m_Velocity(), m_Acceleration() 
{
	ValidateAABB(m_AABB);
}

PhysicsBodyData::PhysicsBodyData(const Physics::AABB& boundingBox)
	: ComponentData(), m_AABB(boundingBox), m_Velocity(), m_Acceleration() 
{
	ValidateAABB(m_AABB);
}

bool PhysicsBodyData::ValidateAABB(const Physics::AABB& bounding) const
{
	const Utils::Point2DInt size = bounding.GetSize();
	if (Utils::Assert(this, size.m_X!=0 && size.m_Y!=0, 
		std::format("Tried to create a Physics Body but the AABB cannot have 0 x or y size: {}", size.ToString()))) 
		return false;

	return true;
}
	

void PhysicsBodyData::SetVelocity(const Vec2& vel)
{
	m_Velocity = vel;
}

void PhysicsBodyData::SetVelocityXDelta(const float& xDelta)
{
	m_Velocity.m_X += xDelta;
}
void PhysicsBodyData::SetVelocityYDelta(const float& yDelta)
{
	m_Velocity.m_Y += yDelta;
}

void PhysicsBodyData::SetAcceleration(const Vec2& acc)
{
	m_Acceleration = acc;
}

const Vec2& PhysicsBodyData::GetVelocity() const
{
	return m_Velocity;
}

const Vec2& PhysicsBodyData::GetAcceleration() const
{
	return m_Acceleration;
}

const Physics::AABB& PhysicsBodyData::GetAABB() const
{
	return m_AABB;
}