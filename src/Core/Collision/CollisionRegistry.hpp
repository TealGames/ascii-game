#pragma once
#include "ECS/Component/Types/World/CollisionBoxComponent.hpp"
#include "Core/Primitives/Direction.hpp"
#include <unordered_map>
#include <string>

namespace Engine::Physics
{
	struct CollisionPair
	{
		CollisionBoxComponent* m_CollisionBoxA;
		CollisionBoxComponent* m_CollisionBoxB;
		AABBIntersectionData m_IntersectionData;
		MoveDirection m_Direction = MoveDirection::North;

		CollisionPair();
		CollisionPair(CollisionBoxComponent& boxA, CollisionBoxComponent& boxB,
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
		std::string CreateCollisionKey(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB) const;
		//CollidingBodiesCollection::iterator TryGetCollisionIteratorMutable(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB);
		//CollidingBodiesCollection::iterator TryGetCollisionIteratorMutable(const std::string& collisionKey);

		CollidingBodiesCollection::const_iterator TryGetCollisionIterator(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB) const;
	public:
		CollisionRegistry();

		bool HasCollision(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB) const;
		bool HasCollision(const CollisionBoxComponent& boxA) const;

		bool TryAddCollision(const CollisionPair& collisionPair);
		bool TryRemoveCollision(const CollisionPair& collisionPair);
		bool TryRemoveCollision(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB);

		const CollisionPair* TryGetCollision(const CollisionBoxComponent& boxA, const CollisionBoxComponent& boxB) const;
		std::vector<const CollisionPair*> TryGetCollisions(const CollisionBoxComponent& box) const;
		std::vector<CollisionPair*> TryGetCollisionsMutable(const CollisionBoxComponent& box);

		int GetCollisionsCount(const CollisionBoxComponent& box) const;

		bool IsCollidingInDirs(const CollisionBoxComponent& box, const std::vector<MoveDirection>& dirs, const bool requireTouch = false) const;
		std::vector<MoveDirection> TryGetCollisionDirs(const CollisionBoxComponent& box) const;
		//const bool& IsCollidingWithAnyBody() const;

		//CollidingBodiesCollection::iterator GetCollidingBodyIterator(const CollisionBoxComponent& collisionBox);
		//bool IsValidCollidingBodyIterator(const CollidingBodiesCollection::iterator& removeBodyIterator) const;
		//CollisionBoxComponent* TryGetCollidingBody(const CollisionBoxComponent& collisionBox);
		//bool IsCollidingWithBody(const CollisionBoxComponent& collisionBox);
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
}