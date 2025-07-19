#pragma once
#include "ECS/Component/Types/World/CollisionBoxData.hpp"
#include <unordered_map>
#include <string>

struct CollisionPair
{
	CollisionBoxData* m_CollisionBoxA;
	CollisionBoxData* m_CollisionBoxB;
	AABBIntersectionData m_IntersectionData;
	MoveDirection m_Direction = MoveDirection::North;

	CollisionPair();
	CollisionPair(CollisionBoxData& boxA, CollisionBoxData& boxB, 
		const AABBIntersectionData& data, const MoveDirection& moveDir);

	std::string ToString() const;
};

//TODO: perhaps find a way to use uint64 or similar for the keys (but then collision box system needs to store collision keys since 
//then they can not be determined just from the name)
using CollidingBodiesCollection = std::unordered_map<std::string, CollisionPair>;
class CollisionRegistry
{
private:
private:
	CollidingBodiesCollection m_collisions;
public:

private:
	std::string CreateCollisionKey(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const;
	//CollidingBodiesCollection::iterator TryGetCollisionIteratorMutable(const CollisionBoxData& boxA, const CollisionBoxData& boxB);
	//CollidingBodiesCollection::iterator TryGetCollisionIteratorMutable(const std::string& collisionKey);

	CollidingBodiesCollection::const_iterator TryGetCollisionIterator(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const;
public:
	CollisionRegistry();

	bool HasCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const;
	bool HasCollision(const CollisionBoxData& boxA) const;

	bool TryAddCollision(const CollisionPair& collisionPair);
	bool TryRemoveCollision(const CollisionPair& collisionPair);
	bool TryRemoveCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB);

	const CollisionPair* TryGetCollision(const CollisionBoxData& boxA, const CollisionBoxData& boxB) const;
	std::vector<const CollisionPair*> TryGetCollisions(const CollisionBoxData& box) const;
	std::vector<CollisionPair*> TryGetCollisionsMutable(const CollisionBoxData& box);

	int GetCollisionsCount(const CollisionBoxData& box) const;

	bool IsCollidingInDirs(const CollisionBoxData& box, const std::vector<MoveDirection>& dirs, const bool requireTouch=false) const;
	std::vector<MoveDirection> TryGetCollisionDirs(const CollisionBoxData& box) const;
	//const bool& IsCollidingWithAnyBody() const;

	//CollidingBodiesCollection::iterator GetCollidingBodyIterator(const CollisionBoxData& collisionBox);
	//bool IsValidCollidingBodyIterator(const CollidingBodiesCollection::iterator& removeBodyIterator) const;
	//CollisionBoxData* TryGetCollidingBody(const CollisionBoxData& collisionBox);
	//bool IsCollidingWithBody(const CollisionBoxData& collisionBox);
	int GetTotalCollisionsCount();
	void ExecuteOnAllCollisions(const std::function<void(CollisionPair&)>& collision);
	/// <summary>
	/// Returns the objects that this object collides with while preserving the order 
	/// the collisions would have happened
	/// </summary>
	/// <returns></returns>
	//const CollidingBodiesCollection& GetCollidingBodiesOrdered() const;

	void ClearAll();

	std::string ToStringCollidingBodies() const;
};