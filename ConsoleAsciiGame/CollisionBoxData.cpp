#include "pch.hpp"
#include "CollisionBoxData.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "TransformData.hpp"
#include "Entity.hpp"
#include "JsonSerializers.hpp"

AABBIntersectionData::AABBIntersectionData() :
	AABBIntersectionData(false, Vec2::ZERO) {}

AABBIntersectionData::AABBIntersectionData(const bool& intersect, const Vec2& depth) :
	m_DoIntersect(intersect), m_Depth(depth) {}

CollisionBoxData::CollisionBoxData(const TransformData* transform, const Vec2& size, const WorldPosition& transformOffset) :
	ComponentData(), m_transform(transform), m_aabb(size), m_transformOffset(transformOffset) {}

CollisionBoxData::CollisionBoxData(const TransformData& transform, const Vec2& size, const WorldPosition& transformOffset) :
	CollisionBoxData(&transform, size, transformOffset) {}

CollisionBoxData::CollisionBoxData() :
	CollisionBoxData(nullptr, {}, {}) {}

CollisionBoxData::CollisionBoxData(const Json& json) : CollisionBoxData()
{
	Deserialize(json);
}

bool CollisionBoxData::operator==(const CollisionBoxData& other) const
{
	return m_transform == other.m_transform;
}

std::vector<std::string> CollisionBoxData::GetDependencyFlags() const
{
	return {Utils::GetTypeName<TransformData>()};
}
void CollisionBoxData::InitFields()
{
	m_Fields = {ComponentField("Offset", &m_transformOffset)};
}

bool CollisionBoxData::Validate()
{
	const Vec2 size = GetAABB().GetSize();
	if (Assert(this, size.m_X != 0 && size.m_Y != 0,
		std::format("Tried to create a Collision box but the AABB cannot have 0 x or y size: {}. "
			"This could be due to bad bounding size or offset!", size.ToString())))
		return false;

	return true;
}

const TransformData& CollisionBoxData::GetTransform() const
{
	if (!Assert(this, m_transform!=nullptr, std::format("Tried to get transform data but was NULL")))
		throw std::invalid_argument("Invalid transform data");

	return *m_transform;
}
const WorldPosition& CollisionBoxData::GetOffset() const
{
	return m_transformOffset;
}
const WorldPosition& CollisionBoxData::GetCurrentPos() const
{
	return GetTransform().m_Pos;
}
const WorldPosition& CollisionBoxData::GetGlobalMin() const
{
	return m_aabb.GetGlobalMin(GetAABBCenterWorldPos());
}
const WorldPosition& CollisionBoxData::GetGlobalMax() const
{
	return m_aabb.GetGlobalMax(GetAABBCenterWorldPos());
}

const Physics::AABB& CollisionBoxData::GetAABB() const
{
	return m_aabb;
}
const WorldPosition CollisionBoxData::GetAABBCenterWorldPos() const
{
	return GetCurrentPos() + m_transformOffset;
}
const WorldPosition CollisionBoxData::GetAABBWorldPos(const NormalizedPosition& relativePos) const
{
	return m_aabb.GetWorldPos(GetAABBCenterWorldPos(), relativePos);
}

const WorldPosition CollisionBoxData::GetAABBTopLeftWorldPos() const
{
	return GetAABBWorldPos(Vec2{ 0, 1 });
}

bool CollisionBoxData::DoIntersect(const WorldPosition& pos) const
{
	WorldPosition minPos = m_aabb.GetGlobalMin(GetAABBCenterWorldPos());
	WorldPosition maxPos = m_aabb.GetGlobalMax(GetAABBCenterWorldPos());

	return minPos.m_X <= pos.m_X && pos.m_X <= maxPos.m_X &&
		minPos.m_Y <= pos.m_Y && pos.m_Y <= maxPos.m_Y;
}

bool CollisionBoxData::DoIntersect(const CollisionBoxData& otherBox) const
{
	return GetCollisionIntersectionData(otherBox).m_DoIntersect;
}

AABBIntersectionData CollisionBoxData::GetCollisionIntersectionData(const CollisionBoxData& otherBox) const
{
	const WorldPosition& thisMinGlobal = GetGlobalMin();
	const WorldPosition& thisMaxGlobal = GetGlobalMax();
	const WorldPosition& otherMinGlobal = otherBox.GetGlobalMin();
	const WorldPosition& otherMaxGlobal = otherBox.GetGlobalMax();

	AABBIntersectionData result = {};
	result.m_DoIntersect = (thisMaxGlobal.m_X > otherMinGlobal.m_X && thisMinGlobal.m_X < otherMaxGlobal.m_X &&
		thisMaxGlobal.m_Y > otherMinGlobal.m_Y && thisMinGlobal.m_Y < otherMaxGlobal.m_Y);

	//Covers case where body2 is fully inside body1
	if (result.m_DoIntersect && thisMinGlobal.m_X <= otherMinGlobal.m_X && otherMaxGlobal.m_X <= thisMaxGlobal.m_X &&
		thisMinGlobal.m_Y <= otherMinGlobal.m_Y && otherMaxGlobal.m_Y <= thisMaxGlobal.m_Y)
	{
		result.m_Depth.m_X = Utils::MinAbs(otherMinGlobal.m_X - thisMaxGlobal.m_X, otherMaxGlobal.m_X - thisMinGlobal.m_X);
		result.m_Depth.m_Y = Utils::MinAbs(otherMinGlobal.m_Y - thisMaxGlobal.m_Y, otherMaxGlobal.m_Y - thisMinGlobal.m_Y);

		/*LogError(std::format("Calculating min abs X of {} and {} is: {} MIN DEPTH Y: {}", std::to_string(otherMinGlobal.m_X - thisMinGlobal.m_X),
			std::to_string(otherMaxGlobal.m_X - thisMaxGlobal.m_X), std::to_string(result.m_Depth.m_X), std::to_string(result.m_Depth.m_Y)));*/
		return result;
	}

	//TODO: intersection vector depth might not work if smaller collider is fully inside bigger collider
	if (result.m_DoIntersect)
	{
		//Coming from right side
		if (otherMaxGlobal.m_X > thisMaxGlobal.m_X)
		{
			result.m_Depth.m_X = otherMinGlobal.m_X - thisMaxGlobal.m_X;
		}
		//Coming from bottom side
		else if (otherMinGlobal.m_X < thisMinGlobal.m_X)
		{
			result.m_Depth.m_X = otherMaxGlobal.m_X - thisMinGlobal.m_X;
		}

		//Coming in from top
		if (otherMaxGlobal.m_Y > thisMaxGlobal.m_Y) result.m_Depth.m_Y = otherMinGlobal.m_Y - thisMaxGlobal.m_Y;
		//Coming from bottom side
		else if (otherMinGlobal.m_Y < thisMinGlobal.m_Y) result.m_Depth.m_Y = otherMaxGlobal.m_Y - thisMinGlobal.m_Y;
	}
	return result;
}

Vec2 CollisionBoxData::GetAABBMinDisplacement(const CollisionBoxData& otherBox) const
{
	const WorldPosition& thisMinGlobal = GetGlobalMin();
	const WorldPosition& thisMaxGlobal = GetGlobalMax();
	const WorldPosition& otherMinGlobal = otherBox.GetGlobalMin();
	const WorldPosition& otherMaxGlobal = otherBox.GetGlobalMax();

	Vec2 displacement = {};

	//body 2 is fully to the RIGHT of body1
	if (thisMaxGlobal.m_X < otherMaxGlobal.m_X && thisMaxGlobal.m_X < otherMinGlobal.m_X) displacement.m_X = otherMinGlobal.m_X - thisMaxGlobal.m_X;
	//body2 is fully to the LEFT of body1
	else if (otherMaxGlobal.m_X < thisMinGlobal.m_X && otherMinGlobal.m_X < thisMinGlobal.m_X) displacement.m_X = otherMaxGlobal.m_X - thisMinGlobal.m_X;
	//If body2 is touching body1 or is only in vertical dir then displacement is 0
	else displacement.m_X = 0;

	// body 2 is fully ON TOP of body1
	if (thisMaxGlobal.m_Y < otherMaxGlobal.m_Y && thisMaxGlobal.m_Y < otherMinGlobal.m_Y) displacement.m_Y = otherMinGlobal.m_Y - thisMaxGlobal.m_Y;
	// body 2 is fully BELOW body1
	else if (otherMaxGlobal.m_Y < thisMinGlobal.m_Y && otherMinGlobal.m_Y < thisMinGlobal.m_Y) displacement.m_Y = otherMaxGlobal.m_Y - thisMinGlobal.m_Y;
	else displacement.m_Y = 0;

	return displacement;
}

Vec2 CollisionBoxData::GetAABBDirection(const CollisionBoxData& otherBox, const bool& considerCollisions) const
{
	const WorldPosition& thisMinGlobal = GetGlobalMin();
	const WorldPosition& thisMaxGlobal = GetGlobalMax();
	const WorldPosition& otherMinGlobal = otherBox.GetGlobalMin();
	const WorldPosition& otherMaxGlobal = otherBox.GetGlobalMax();

	Vec2 dir = Vec2::ZERO;
	const WorldPosition& thisCenter = GetAABBCenterWorldPos();
	if (considerCollisions)
	{
		//If enttiy 2 is to the right of entity1 center x
		if (thisCenter.m_X < otherMaxGlobal.m_X && thisCenter.m_X <= otherMinGlobal.m_X) dir.m_X = 1;
		//Entity 2 is to the left of entity1 center x
		else if (otherMinGlobal.m_X < GetAABBCenterWorldPos().m_X && otherMaxGlobal.m_X <= thisCenter.m_X) dir.m_X = -1;

		//Entity2 is on top of entity1 center y
		if (thisCenter.m_Y < otherMaxGlobal.m_Y && thisCenter.m_Y <= otherMinGlobal.m_Y) dir.m_Y = 1;
		//Entity2 is bottom of entity1 center y
		else if (otherMinGlobal.m_Y < thisCenter.m_Y && otherMaxGlobal.m_Y <= thisCenter.m_Y) dir.m_Y = -1;
	}
	else
	{
		//If enttiy 2 is to the right FULLY
		if (thisMaxGlobal.m_X < otherMaxGlobal.m_X && thisMaxGlobal.m_X < otherMinGlobal.m_X) dir.m_X = 1;
		//Entity 2 is to the left of entity 1 FULLY
		else if (otherMinGlobal.m_X < thisMinGlobal.m_X && otherMaxGlobal.m_X < thisMinGlobal.m_X) dir.m_X = -1;

		//Entity2 is on top of entity1 FULLY
		if (thisMaxGlobal.m_Y < otherMaxGlobal.m_Y && thisMaxGlobal.m_Y < otherMinGlobal.m_Y) dir.m_Y = 1;
		//Entity2 is bottom of entity1 FULLY
		else if (otherMinGlobal.m_Y < thisMinGlobal.m_Y && otherMaxGlobal.m_Y < thisMinGlobal.m_Y) dir.m_Y = -1;
	}
	/*if (thisMaxGlobal.m_X < otherMaxGlobal.m_X && thisMaxGlobal.m_X <= otherMinGlobal.m_X) return Direction::Right;
	else if (otherMinGlobal.m_X < thisMinGlobal.m_X && otherMaxGlobal.m_X <= thisMinGlobal.m_X) return Direction::Left;
	else if (thisMaxGlobal.m_Y < otherMaxGlobal.m_Y && thisMaxGlobal.m_Y <= otherMinGlobal.m_Y) return Direction::Up;
	else if (otherMinGlobal.m_Y < thisMinGlobal.m_Y && otherMaxGlobal.m_Y <= thisMinGlobal.m_Y) return Direction::Down;
	else
	{
		LogError(std::format("Tried to get AABB direction but no directions match any of the criteria! Entity 1:{} Entity2:{}",
			entity1Bounding.ToString(entity1Pos), entity2Bounding.ToString(entity2Pos)));
		return std::nullopt;
	}*/

	if (Utils::ApproximateEqualsF(std::abs(dir.m_X), 1) &&
		Utils::ApproximateEqualsF(std::abs(dir.m_Y), 1))
	{
		dir.m_X = Utils::GetSign(dir.m_X) * 0.5;
		dir.m_Y = Utils::GetSign(dir.m_Y) * 0.5;
	}

	//Assert(false, std::format("aabb dir: {} normalized: {}", dir.ToString(), dir.GetNormalized().ToString()));
	return dir;
}

std::string CollisionBoxData::ToString() const
{
	return std::format("[CollisionBox AABB:{}, Offset:{} transform:{}]", m_aabb.ToString(GetAABBCenterWorldPos()), 
		m_transformOffset.ToString(), m_transform!=nullptr? GetTransform().ToString() : "NULL");
}

void CollisionBoxData::Deserialize(const Json& json)
{
	m_aabb = json.at("AABB").get<Physics::AABB>();
	m_transformOffset = json.at("Offset").get<WorldPosition>();
	m_transform = &(GetEntitySafe().m_Transform);
}
Json CollisionBoxData::Serialize()
{
	return Json{ {"AABB", m_aabb}, {"Offset", m_transformOffset}};
}
