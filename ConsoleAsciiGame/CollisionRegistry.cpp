#include "pch.hpp"
#include "CollisionRegistry.hpp"
#include "Entity.hpp"
#include "Debug.hpp"

static constexpr char COLLISION_KEY_SEPARATOR = '|';

CollisionPair::CollisionPair() : 
	m_CollisionBoxA(nullptr), m_CollisionBoxB(nullptr), m_IntersectionData(), m_Direction(MoveDirection::North) {}

CollisionPair::CollisionPair(CollisionBoxData& boxA, CollisionBoxData& boxB,
	const AABBIntersectionData& data, const MoveDirection& moveDir) 
	: m_CollisionBoxA(&boxA), m_CollisionBoxB(&boxB), m_IntersectionData(data), m_Direction(moveDir) {}

CollisionRegistry::CollisionRegistry() {}

std::string CollisionRegistry::CreateCollisionKey(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const
{
	return boxA.GetEntitySafe().GetName() + COLLISION_KEY_SEPARATOR + boxB.GetEntitySafe().GetName();
}

/*
CollidingBodiesCollection::iterator CollisionRegistry::TryGetCollisionIteratorMutable(const CollisionBoxData& boxA, const CollisionBoxData& boxB)
{
	const std::string possibleKey1 = CreateCollisionKey(boxA, boxB);
	auto maybeIt = m_collisions.find(possibleKey1);
	if (maybeIt != m_collisions.end())
	{
		return maybeIt;
	}
	
	//To ensure we find valid key either box order we search both options
	const std::string possibleKey2 = CreateCollisionKey(boxB, boxA);
	maybeIt= m_collisions.find(possibleKey2);
	return maybeIt;
}
CollidingBodiesCollection::iterator CollisionRegistry::TryGetCollisionIteratorMutable(const std::string& collisionKey)
{
	return m_collisions.find(collisionKey);
}
*/

CollidingBodiesCollection::const_iterator CollisionRegistry::TryGetCollisionIterator(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const
{
	const std::string possibleKey1 = CreateCollisionKey(boxA, boxB);
	auto maybeIt = m_collisions.find(possibleKey1);
	if (maybeIt != m_collisions.end())
	{
		return maybeIt;
	}

	//To ensure we find valid key either box order we search both options
	const std::string possibleKey2 = CreateCollisionKey(boxB, boxA);
	maybeIt = m_collisions.find(possibleKey2);
	return maybeIt;
}

bool CollisionRegistry::HasCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const
{
	if (m_collisions.empty()) return false;

	auto maybeIt = TryGetCollisionIterator(boxA, boxB);
	return maybeIt != m_collisions.end();
}

bool CollisionRegistry::HasCollision(const CollisionBoxData& box) const
{
	if (m_collisions.empty()) return false;

	//TODO: can be optimized to have collisions in alphetical order and search by letters
	for (auto& collision : m_collisions)
	{
		if (*collision.second.m_CollisionBoxA == box || *collision.second.m_CollisionBoxB == box)
			return true;
	}
	return false;
}

bool CollisionRegistry::TryAddCollision(const CollisionPair& pair)
{
	if (!Assert(this, pair.m_CollisionBoxA != nullptr && pair.m_CollisionBoxB != nullptr,
		std::format("Tried to create add collision to registry but either one or both collision boxes are NULL")))
		return false;

	if (HasCollision(*pair.m_CollisionBoxA, *pair.m_CollisionBoxB))
		return false;

	m_collisions.emplace(CreateCollisionKey(*pair.m_CollisionBoxA, *pair.m_CollisionBoxB), pair);
	return true;
}

bool CollisionRegistry::TryRemoveCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB)
{
	auto maybeIt = TryGetCollisionIterator(boxA, boxB);
	if (maybeIt == m_collisions.end()) return false;

	m_collisions.erase(maybeIt);
	return true;
}

const CollisionPair* CollisionRegistry::TryGetCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const
{
	auto maybeIt= TryGetCollisionIterator(boxA, boxB);
	if (maybeIt == m_collisions.end()) return nullptr;

	return &(maybeIt->second);
}
std::vector<const CollisionPair*> CollisionRegistry::TryGetCollisions(const CollisionBoxData& box) const
{
	if (m_collisions.empty()) return {};

	std::vector<const CollisionPair*> collisions = {};
	for (const auto& collision : m_collisions)
	{
		if (*collision.second.m_CollisionBoxA == box || *collision.second.m_CollisionBoxB == box)
			collisions.push_back(&(collision.second));
	}
	return collisions;
}
std::vector<CollisionPair*> CollisionRegistry::TryGetCollisionsMutable(const CollisionBoxData& box)
{
	if (m_collisions.empty()) return {};

	std::vector<CollisionPair*> collisions = {};
	for (auto& collision : m_collisions)
	{
		if (*collision.second.m_CollisionBoxA == box || *collision.second.m_CollisionBoxB == box)
			collisions.push_back(&(collision.second));
	}
	return collisions;
}

bool CollisionRegistry::IsCollidingInDirs(const CollisionBoxData& box, const std::vector<MoveDirection>& dirs) const
{
	if (m_collisions.empty()) return false;

	auto collisions = TryGetCollisions(box);
	if (collisions.empty()) return false;

	for (const auto& collision : collisions)
	{
		if (collision == nullptr) continue;
		if (std::find(dirs.begin(), dirs.end(), collision->m_Direction) != dirs.end())
			return true;
	}
	return false;
}
int CollisionRegistry::GetTotalCollisions()
{
	return m_collisions.size();
}
std::string CollisionRegistry::ToStringCollidingBodies() const
{
	return "";
	/*if (m_collidingBodies.empty()) return "[]";
	std::string bodiesStr = "[";

	for (const auto& currentObj : m_collidingBodies)
	{
		if (currentObj.m_CollisionBox == nullptr) continue;
		bodiesStr += std::format("{},", currentObj.m_CollisionBox->GetEntitySafe().ToString());
	}
	bodiesStr += "]";
	return bodiesStr;*/
}


