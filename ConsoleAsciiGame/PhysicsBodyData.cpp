#include "pch.hpp"
#include "PhysicsBodyData.hpp"
#include "HelperFunctions.hpp"
#include "Entity.hpp"
#include <limits>

PhysicsBodyData::PhysicsBodyData() : 
	PhysicsBodyData({}, {}, 0, std::numeric_limits<float>::max()) {}

PhysicsBodyData::PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset) : 
	PhysicsBodyData(boundingBoxSize, transformOffset, 0, 0) {}

PhysicsBodyData::PhysicsBodyData(const Utils::Point2D& boundingBoxSize, const WorldPosition& transformOffset, const float& gravity, const float& terminalYVelocity)
	: ComponentData(), m_aabb(CreateAABB(boundingBoxSize, transformOffset)), 
	m_velocity(), m_acceleration(), m_transformOffset(transformOffset), m_collidingBodies(), 
	m_gravity(-std::abs(gravity)), m_terminalYVelocity(-std::abs(terminalYVelocity))
{
	//Utils::LogWarning(std::format("Created physics body of size: {} offset: {} that has min: {} max: {} size: {}",
	//boundingBoxSize.ToString(), transformOffset.ToString(), m_AABB.m_MinPos.ToString(), 
	//m_AABB.m_MaxPos.ToString(), m_AABB.GetSize().ToString());

	ValidateAABB(m_aabb);
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
	
void PhysicsBodyData::SetVelocityXDelta(const float& xDelta)
{
	SetVelocityDelta({ xDelta, 0 });
}
void PhysicsBodyData::SetVelocityYDelta(const float& yDelta)
{
	SetVelocityDelta({0, yDelta});
}
void PhysicsBodyData::SetVelocityDelta(const Vec2& vel)
{
	SetVelocity(m_velocity + vel);
}
void PhysicsBodyData::SetVelocity(const Vec2& vel)
{
	m_velocity.m_X = vel.m_X;
	m_velocity.m_Y = std::max(vel.m_Y, m_terminalYVelocity);
}

void PhysicsBodyData::SetAcceleration(const Vec2& acc)
{
	m_acceleration = acc;
}

const Vec2& PhysicsBodyData::GetVelocity() const
{
	return m_velocity;
}

const Vec2& PhysicsBodyData::GetAcceleration() const
{
	return m_acceleration;
}

const float& PhysicsBodyData::GetGravity() const
{
	return m_gravity;
}

const Physics::AABB& PhysicsBodyData::GetAABB() const
{
	return m_aabb;
}

const WorldPosition PhysicsBodyData::GetAABBTopLeftWorldPos(const WorldPosition& currentPos) const
{
	const Utils::Point2D aabbHalfExtent = m_aabb.GetHalfExtent();
	return currentPos + m_transformOffset + WorldPosition(-aabbHalfExtent.m_X, aabbHalfExtent.m_Y);
}

void PhysicsBodyData::AddCollidingBody(PhysicsBodyData& collidingBody)
{
	m_collidingBodies.push_back(&collidingBody);
}
void PhysicsBodyData::RemoveCollidingBody(const std::vector<PhysicsBodyData*>::iterator& removeBodyIterator)
{
	m_collidingBodies.erase(removeBodyIterator);
}

bool PhysicsBodyData::IsValidCollidingBodyIterator(const std::vector<PhysicsBodyData*>::iterator& removeBodyIterator) const
{
	return removeBodyIterator != m_collidingBodies.end();
}
std::vector<PhysicsBodyData*>::iterator PhysicsBodyData::GetCollidingBodyIterator(const PhysicsBodyData& physicsBody)
{
	if (!IsCollidingWithAnyBody()) return m_collidingBodies.end();

	PhysicsBodyData* body = nullptr;
	for (int i=0; i<m_collidingBodies.size(); i++)
	{
		body = m_collidingBodies[i];
		if (body == nullptr) continue;
		//TODO: possible problem because globals and locals are stored separately so there could be 
		//a local and global with same id
		if (body->GetEntitySafeMutable().m_Id == physicsBody.GetEntitySafe().m_Id &&
			body->GetEntitySafeMutable().m_Name == physicsBody.GetEntitySafe().m_Name)
		{
			return m_collidingBodies.begin() + i;
		}
	}
	return m_collidingBodies.end();
}
const bool& PhysicsBodyData::IsCollidingWithAnyBody() const
{
	return !m_collidingBodies.empty();
}
PhysicsBodyData* PhysicsBodyData::TryGetCollidingBody(const PhysicsBodyData& physicsBody)
{
	auto it = GetCollidingBodyIterator(physicsBody);
	if (it == m_collidingBodies.end()) return nullptr;
	return *it;
}
bool PhysicsBodyData::IsCollidingWithBody(const PhysicsBodyData& physicsBody)
{
	return TryGetCollidingBody(physicsBody) != nullptr;
}
int PhysicsBodyData::GetTotalBodyCollisions()
{
	return m_collidingBodies.size();
}
std::string PhysicsBodyData::ToStringCollidingBodies() const
{
	if (m_collidingBodies.empty()) return "[]";
	std::string bodiesStr = "[";

	for (const auto& body : m_collidingBodies)
	{
		if (body == nullptr) continue;
		bodiesStr += std::format("{},", body->GetEntitySafe().ToString());
	}
	bodiesStr += "]";
	return bodiesStr;
}