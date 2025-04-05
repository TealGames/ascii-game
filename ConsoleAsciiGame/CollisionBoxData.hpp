#pragma once
#include "AABB.hpp"
#include "ComponentData.hpp"
#include "IValidateable.hpp"

struct AABBIntersectionData
{
	bool m_DoIntersect;
	Vec2 m_Depth;

	AABBIntersectionData();
	AABBIntersectionData(const bool& intersect, const Vec2& depth);
};

class TransformData;
class CollisionBoxData : public ComponentData
{
private:
	Physics::AABB m_aabb;
	WorldPosition m_transformOffset;
	const TransformData* m_transform;

	//CollidingBodiesCollection m_collidingBodies;

public:
	static constexpr float MAX_DISTANCE_FOR_COLLISION = 0.01;

private:
	bool Validate() override;
	const TransformData& GetTransform() const;
	const WorldPosition& GetCurrentPos() const;

	const WorldPosition& GetGlobalMin() const;
	const WorldPosition& GetGlobalMax() const;

	CollisionBoxData(const TransformData* transform, const Vec2& size, const WorldPosition& transformOffset);

public:
	CollisionBoxData();
	CollisionBoxData(const Json& json);
	CollisionBoxData(const TransformData& transform, const Vec2& size, const WorldPosition& transformOffset);

	bool operator==(const CollisionBoxData& other) const;

	const Physics::AABB& GetAABB() const;
	const WorldPosition& GetOffset() const;

	//bool HasValidTransform() const;
	//void SetTransform(const TransformData& transform);
	const WorldPosition GetAABBCenterWorldPos() const;
	const WorldPosition GetAABBTopLeftWorldPos() const;

	/// <summary>
	/// Will get the AABB pos based on the relative pos of the AABB
	/// where (0,0) is bottom left and (1, 1) is top right
	/// </summary>
	/// <param name="relativePos"></param>
	/// <returns></returns>
	const WorldPosition GetAABBWorldPos(const NormalizedPosition& relativePos) const;

	bool DoIntersect(const WorldPosition& pos) const;
	bool DoIntersect(const CollisionBoxData& otherBox) const;

	AABBIntersectionData GetCollisionIntersectionData(const CollisionBoxData& otherBox) const;

	/// <summary>
	/// Will get the minimum displacement that aabb2 is from aabb1 
	/// Note: if they are intersecting displacement will be 0
	/// </summary>
	/// <param name="entity1Pos"></param>
	/// <param name="entity1Bounding"></param>
	/// <param name="entity2Pos"></param>
	/// <param name="entity2Bounding"></param>
	/// <returns></returns>
	Vec2 GetAABBMinDisplacement(const CollisionBoxData& otherBox) const;

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
	Vec2 GetAABBDirection(const CollisionBoxData& otherBox, const bool& considerCollisions = true) const;

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

