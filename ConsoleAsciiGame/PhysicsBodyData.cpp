#include "pch.hpp"
#include "PhysicsBodyData.hpp"
#include "HelperFunctions.hpp"

PhysicsBodyData::PhysicsBodyData() 
	: ComponentData(), m_AABB(),  m_Velocity(), m_Acceleration(), m_TransformOffset()
{
	ValidateAABB(m_AABB);
}

PhysicsBodyData::PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset)
	: ComponentData(), m_AABB(CreateAABB(boundingBoxSize, transformOffset)), 
	m_Velocity(), m_Acceleration(), m_TransformOffset(transformOffset)
{
	//Utils::LogWarning(std::format("Created physics body of size: {} offset: {} that has min: {} max: {} size: {}",
	//boundingBoxSize.ToString(), transformOffset.ToString(), m_AABB.m_MinPos.ToString(), 
	//m_AABB.m_MaxPos.ToString(), m_AABB.GetSize().ToString());

	ValidateAABB(m_AABB);
}

bool PhysicsBodyData::ValidateAABB(const Physics::AABB& bounding) const
{
	const Utils::Point2D size = bounding.GetSize();
	if (Utils::Assert(this, size.m_X!=0 && size.m_Y!=0, 
		std::format("Tried to create a Physics Body but the AABB cannot have 0 x or y size: {}. "
			"This could be due to bad bounding size or offset!", size.ToString()))) 
		return false;

	return true;
}

Physics::AABB PhysicsBodyData::CreateAABB(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset)
{
	return {transformOffset- (boundingBoxSize/2), transformOffset+ (boundingBoxSize / 2) };
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

const WorldPosition PhysicsBodyData::GetAABBTopLeftWorldPos(const WorldPosition& currentPos) const
{
	const Utils::Point2D aabbHalfExtent = m_AABB.GetHalfExtent();
	return currentPos + m_TransformOffset + WorldPosition(-aabbHalfExtent.m_X, aabbHalfExtent.m_Y);
}