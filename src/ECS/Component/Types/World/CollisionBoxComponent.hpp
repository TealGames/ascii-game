#pragma once
#include <cstdint>
#include "Core/Primitives/AABB.hpp"
#include "ECS/Component/Component.hpp"
#include "Core/IValidateable.hpp"
#include <unordered_map>

namespace Engine::Physics
{
	struct AABBIntersectionData
	{
		bool m_DoIntersect;
		/// <summary>
		/// The penetration vector of bodyB into bodyA
		/// </summary>
		Vec2 m_Depth;

		AABBIntersectionData();
		AABBIntersectionData(const bool& intersect, const Vec2& depth);

		bool IsTouchingIntersection() const;

		std::string ToString() const;
	};

	enum class CollisionFlag : std::uint8_t
	{
		None = 0,
		AddedThisFrame = 1,
		RemovedThisFrame = 2,
	};
	std::string ToString(const CollisionFlag flag);

	class CollisionBoxComponent;
	struct CollidingBoxInfo
	{
		const CollisionBoxComponent* m_Box;
		CollisionFlag m_Flag;

		CollidingBoxInfo(const CollisionBoxComponent& box, const CollisionFlag& flag);
	};

	class TransformComponent;
	using CollidingInfoCollection = std::vector<CollidingBoxInfo>;
	class CollisionBoxComponent : public ECS::Component
	{
	private:
		AABB2D m_aabb;
		/// <summary>
		/// The amount of units in local space of the AABB's center from this object's world transform position
		/// </summary>
		WorldPosition2D m_aabbOffset;

		CollidingInfoCollection m_collidingBoxes;

		//CollidingBodiesCollection m_collidingBodies;

	public:
		static constexpr float MAX_DISTANCE_FOR_COLLISION = 0.01;

	public:
		bool Validate() override;
		WorldPosition2D GetCenterGlobalPos() const;

		CollidingInfoCollection::iterator TryGetCollidingBoxIt(const CollisionBoxComponent& otherBox);

	public:
		CollisionBoxComponent();
		CollisionBoxComponent(const Vec2& worldSize, const WorldPosition2D& transformOffset);

		bool IsCollidingWithBox(const CollisionBoxComponent& otherBox) const;
		bool TryAddCollidingBox(const CollisionBoxComponent& otherBox);
		bool TryRemoveCollidingBox(const CollisionBoxComponent& otherBox);

		std::vector<const CollisionBoxComponent*> GetCollisionEnterBoxes() const;
		std::vector<const CollisionBoxComponent*> GetCollisionExitBoxes() const;
		std::vector<const CollisionBoxComponent*> GetAllCollisionBoxes() const;

		/// <summary>
		/// Updates the state of the colliding box info. Collisions added this frame are set to the default flag
		/// while those that had a frame to be in the removed frame state will actually get removed
		/// </summary>
		void UpdateCollisionStates();

		bool operator==(const CollisionBoxComponent& other) const;

		const AABB2D& GetAABB() const;
		WorldPosition2D GetOffset() const;

		//bool HasValidTransform() const;
		//void SetTransform(const TransformData& transform);
		WorldPosition2D GetAABBCenterWorldPos() const;
		WorldPosition2D GetGlobalMin() const;
		WorldPosition2D GetGlobalMax() const;

		WorldPosition2D GetAABBTopLeftWorldPos() const;

		/// <summary>
		/// Will get the AABB pos based on the relative pos of the AABB
		/// where (0,0) is bottom left and (1, 1) is top right
		/// </summary>
		/// <param name="relativePos"></param>
		/// <returns></returns>
		WorldPosition2D GetAABBWorldPos(const NormalizedVec2& relativePos) const;

		bool DoIntersect(const WorldPosition2D& pos) const;
		bool DoIntersect(const CollisionBoxComponent& otherBox) const;

		/// <summary>
		/// Returns intersection considering OTHERBOX as colliding with THISBOX
		/// For example depth is how much OTHERBOX is inside THISBOX
		/// </summary>
		/// <param name="otherBox"></param>
		/// <returns></returns>
		AABBIntersectionData GetCollisionIntersectionData(const CollisionBoxComponent& otherBox) const;

		/// <summary>
		/// Will get the minimum displacement that aabb2 is from aabb1 
		/// Note: if they are intersecting displacement will be 0
		/// </summary>
		/// <param name="entity1Pos"></param>
		/// <param name="entity1Bounding"></param>
		/// <param name="entity2Pos"></param>
		/// <param name="entity2Bounding"></param>
		/// <returns></returns>
		Vec2 GetAABBMinDisplacement(const CollisionBoxComponent& otherBox) const;

		/// <summary>
		/// Will get the direction that is the second one in relation to first in normalized dir
		/// NOTE: if collisions considered then it will use the entity1 CENTER as reference for dir whereas
		/// no collisions considered will only count if the entity2 is FULLY outside of entity1 in that direction
		/// Example: 'Background'(Entity1) is at x= 0 and 'Player'(entity2) is at x=1 then result is [1, 0] (RIGHT)
		/// returns [0, 0] if body2 is inside body1
		/// </summary>
		/// <param name="entity1Pos"></param>
		/// <param name="entity1Bounding"></param>
		/// <param name="entity2Pos"></param>
		/// <param name="entity2Bounding"></param>
		/// <returns></returns>
		Vec2 GetAABBDirection(const CollisionBoxComponent& otherBox, const bool& considerCollisions = true) const;

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
		std::string ToStringRelative() const;
	};
}
