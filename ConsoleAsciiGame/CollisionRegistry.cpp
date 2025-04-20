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

std::string CollisionPair::ToString() const
{
	return std::format("[BoxA:{} BoxB:{} dir:{} intersect:{}]", m_CollisionBoxA->ToString(), 
		m_CollisionBoxB->ToString(), ::ToString(m_Direction), m_IntersectionData.ToString());
}

CollisionRegistry::CollisionRegistry() : m_collisions() {}

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

	//We attempt to remove it a collision between the same two objects already exists
	//Note: nothing goes wrong if the collision does not exist
	TryRemoveCollision(pair);

	m_collisions.emplace(CreateCollisionKey(*pair.m_CollisionBoxA, *pair.m_CollisionBoxB), pair);
	return true;
}

bool CollisionRegistry::TryRemoveCollision(const CollisionPair& collisionPair)
{
	if (collisionPair.m_CollisionBoxA == nullptr || collisionPair.m_CollisionBoxB == nullptr) 
		return false;

	return TryRemoveCollision(*collisionPair.m_CollisionBoxA, *collisionPair.m_CollisionBoxB);
}

bool CollisionRegistry::TryRemoveCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB)
{
	auto maybeIt = TryGetCollisionIterator(boxA, boxB);
	if (maybeIt == m_collisions.end()) return false;

	m_collisions.erase(maybeIt);
	return true;
}

//bool CollisionRegistry::ForceUpdateCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB)
//{
//	const CollisionPair* collision = TryGetCollision(boxA, boxB);
//	if (!Assert(this, collision != nullptr, std::format("Tried to force update a collision between A : {} B: {} "
//		"but a collision between those two boxes does not exist", boxA.ToString(), boxB.ToString())))
//		return false;
//
//
//
//}

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

int CollisionRegistry::GetCollisionsCount(const CollisionBoxData& box) const
{
	return TryGetCollisions(box).size();
}

bool CollisionRegistry::IsCollidingInDirs(const CollisionBoxData& box, const std::vector<MoveDirection>& dirs, const bool requireTouch) const
{
	if (m_collisions.empty()) return false;

	auto collisions = TryGetCollisions(box);
	if (collisions.empty()) return false;

	for (const auto& collision : collisions)
	{
		if (collision == nullptr) continue;
		if (std::find(dirs.begin(), dirs.end(), collision->m_Direction) != dirs.end())
		{
			if (requireTouch && !collision->m_IntersectionData.IsTouchingIntersection()) continue;
			return true;
		}
	}
	return false;
}
std::vector<MoveDirection> CollisionRegistry::TryGetCollisionDirs(const CollisionBoxData& box) const
{
	if (m_collisions.empty()) return {};

	auto collisions = TryGetCollisions(box);
	if (collisions.empty()) return {};

	std::vector<MoveDirection> collisionDirs = {};
	for (const auto& collision : collisions)
	{
		if (collision == nullptr) continue;
		collisionDirs.push_back(collision->m_Direction);
	}
	return collisionDirs;
}
int CollisionRegistry::GetTotalCollisionsCount()
{
	return m_collisions.size();
}
void CollisionRegistry::ExecuteOnAllCollisions(const std::function<void(CollisionPair&)>& action)
{
	for (auto& collision : m_collisions)
	{
		action(collision.second);
	}
}

void CollisionRegistry::ClearAll()
{
	m_collisions.clear();
}

std::string CollisionRegistry::ToStringCollidingBodies() const
{
	std::string collisionStr = "[]";

	for (const auto& collision : m_collisions)
	{
		collisionStr += std::format("[Name:{} Data:{}]", collision.first, collision.second.ToString());
	}
	return collisionStr;
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


