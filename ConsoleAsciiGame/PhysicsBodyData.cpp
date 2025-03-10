#include "pch.hpp"
#include "PhysicsBodyData.hpp"
#include "HelperFunctions.hpp"
#include "Entity.hpp"
#include <limits>
#include <optional>
#include "JsonSerializers.hpp"
#include "Debug.hpp"

PhysicsBodyData::PhysicsBodyData() : 
	PhysicsBodyData(0, {}, {}, 0, std::numeric_limits<float>::max()) {}

PhysicsBodyData::PhysicsBodyData(const Json& json) : PhysicsBodyData()
{
	Deserialize(json);
}
PhysicsBodyData::PhysicsBodyData(const float& mass, const Vec2& boundingBoxSize, const WorldPosition& transformOffset) :
	PhysicsBodyData(mass, boundingBoxSize, transformOffset, 0, 0) {}

PhysicsBodyData::PhysicsBodyData(const float& mass, const Vec2& boundingBoxSize, 
	const WorldPosition& transformOffset, const float& gravity, const float& terminalYVelocity)
	: ComponentData(), m_mass(std::abs(mass)), m_aabb(CreateAABB(boundingBoxSize, transformOffset)), 
	m_velocity(), m_acceleration(), m_transformOffset(transformOffset), m_collidingBodies(), 
	m_gravity(-std::abs(gravity)), m_terminalYVelocity(-std::abs(terminalYVelocity)), 
	m_profile(0, 1)
{
	//LogWarning(std::format("Created physics body of size: {} offset: {} that has min: {} max: {} size: {}",
	//boundingBoxSize.ToString(), transformOffset.ToString(), m_AABB.m_MinPos.ToString(), 
	//m_AABB.m_MaxPos.ToString(), m_AABB.GetSize().ToString());

	ValidateAABB(m_aabb);
}

void PhysicsBodyData::InitFields()
{
	m_Fields = 
	{	
		ComponentField("Mass(KG)", &m_mass), ComponentField("Gravity", &m_gravity),
		ComponentField("TerminalYVelocity", &m_terminalYVelocity), ComponentField("Velocity(m/s)", [this](Vec2 vec)-> void {SetVelocity(vec);}, &m_velocity),
		ComponentField("Accel(m/s2)", [this](Vec2 vec)-> void {SetAcceleration(vec); }, &m_acceleration),
		ComponentField("Restitution", (std::function<void(float)>)[this](float restitution)-> void 
						{m_profile.SetRestitution(restitution); }, &(m_profile.GetRestitutionMutable())),
		ComponentField("Friction", (std::function<void(float)>)[this](float friction)-> void 
						{m_profile.SetFriction(friction); }, &(m_profile.GetFrictionMutable())),
	};
}

bool PhysicsBodyData::ValidateAABB(const Physics::AABB& bounding) const
{
	const Vec2 size = bounding.GetSize();
	if (Assert(this, size.m_X!=0 && size.m_Y!=0, 
		std::format("Tried to create a Physics Body but the AABB cannot have 0 x or y size: {}. "
			"This could be due to bad bounding size or offset!", size.ToString()))) 
		return false;

	return true;
}

Physics::AABB PhysicsBodyData::CreateAABB(const Vec2& boundingBoxSize, const WorldPosition& transformOffset)
{
	return {transformOffset- (boundingBoxSize/2), transformOffset+ (boundingBoxSize / 2) };
}

void PhysicsBodyData::SetPhysicsWorld(const Physics::PhysicsWorld& world)
{
	m_physicsSimulation = &world;
}
const Physics::PhysicsWorld& PhysicsBodyData::GetPhysicsWorldSafe()
{
	if (!Assert(this, m_physicsSimulation != nullptr,
		std::format("Tried to get physics world of body: '{}' "
			"but it is NULL", m_Entity->m_Name)))
		throw std::invalid_argument("Failed to retrieve physics world");

	return *m_physicsSimulation;
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

bool PhysicsBodyData::IsExperiencingGravity() const
{
	return Utils::ApproximateEqualsF(m_acceleration.m_Y, m_gravity);
}

const float& PhysicsBodyData::GetMass() const
{
	return m_mass;
}

bool PhysicsBodyData::ConservesMomentum() const
{
	return m_mass > 0;
}
Vec2 PhysicsBodyData::GetMomentum() const
{
	return m_velocity * m_mass;
}
const Physics::PhysicsProfile& PhysicsBodyData::GetPhysicsProfile() const
{
	return m_profile;
}

const Physics::AABB& PhysicsBodyData::GetAABB() const
{
	return m_aabb;
}

const WorldPosition PhysicsBodyData::GetAABBTopLeftWorldPos() const
{
	return GetAABBWorldPos(Vec2{ 0, 1 });	
	/*
	const Vec2 aabbHalfExtent = m_aabb.GetHalfExtent();
	return GetEntitySafe().m_Transform.m_Pos + m_transformOffset + WorldPosition(-aabbHalfExtent.m_X, aabbHalfExtent.m_Y);
	*/
}

const WorldPosition PhysicsBodyData::GetAABBWorldPos(const NormalizedPosition& relativePos) const
{
	return m_aabb.GetWorldPos(GetAABBCenterWorldPos(), relativePos);
}

const WorldPosition PhysicsBodyData::GetAABBCenterWorldPos() const
{
	return GetEntitySafe().m_Transform.m_Pos + m_transformOffset;
}

bool PhysicsBodyData::DoesAABBContainPos(const WorldPosition& pos) const
{
	WorldPosition minPos = m_aabb.GetGlobalMin(GetAABBCenterWorldPos());
	WorldPosition maxPos = m_aabb.GetGlobalMax(GetAABBCenterWorldPos());

	return minPos.m_X <= pos.m_X && pos.m_X <= maxPos.m_X && 
		   minPos.m_Y <= pos.m_Y && pos.m_Y <= maxPos.m_Y;
}

void PhysicsBodyData::AddCollidingBody(PhysicsBodyData& collidingBody)
{
	//Vec2 collidingDir = GetVector(GetEntitySafeMutable().m_Transform.m_Pos, collidingBody.GetEntitySafeMutable().m_Transform.m_Pos);
	
	//Note: although we are using current pos to get dir and pos may change, no matter what direction colliding body goes
	//it should maintain its direction from this body OR it would not be considered a colliding body and should get removed
	Vec2 collidingDir = Physics::GetAABBDirection(GetAABBCenterWorldPos(), GetAABB(),
		collidingBody.GetAABBCenterWorldPos(), collidingBody.GetAABB(), true);
	std::optional<MoveDirection> maybeDirType = TryConvertVectorToDirection(collidingDir);
	if (!Assert(this, maybeDirType.has_value(), std::format("Tried to add colliding body named '{}' "
		"to body: '{}' but could not deduce direction from vector: {} of body2 relative to body1", 
		collidingBody.GetEntitySafe().m_Name, GetEntitySafe().m_Name, collidingDir.ToString())))
	{
		return;
	}

	m_collidingBodies.push_back(CollidingObject(&collidingBody, maybeDirType.value()));
}
void PhysicsBodyData::RemoveCollidingBody(const CollidingBodiesCollection::iterator& removeBodyIterator)
{
	m_collidingBodies.erase(removeBodyIterator);
}

bool PhysicsBodyData::IsValidCollidingBodyIterator(const CollidingBodiesCollection::iterator& removeBodyIterator) const
{
	return removeBodyIterator != m_collidingBodies.end();
}
CollidingBodiesCollection::iterator PhysicsBodyData::GetCollidingBodyIterator(const PhysicsBodyData& physicsBody)
{
	if (!IsCollidingWithAnyBody()) return m_collidingBodies.end();

	CollidingObject currentObj = {};
	for (int i=0; i<m_collidingBodies.size(); i++)
	{
		currentObj = m_collidingBodies[i];
		if (currentObj.m_Body == nullptr) continue;
		//TODO: possible problem because globals and locals are stored separately so there could be 
		//a local and global with same id
		if (currentObj.m_Body->GetEntitySafeMutable().m_Id == physicsBody.GetEntitySafe().m_Id &&
			currentObj.m_Body->GetEntitySafeMutable().m_Name == physicsBody.GetEntitySafe().m_Name)
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
bool PhysicsBodyData::IsCollidingWithBodyInDirs(const std::vector<MoveDirection>& dirs) const
{
	if (!IsCollidingWithAnyBody()) return false;

	CollidingObject currentObj = {};
	for (int i = 0; i < m_collidingBodies.size(); i++)
	{
		currentObj = m_collidingBodies[i];
		if (currentObj.m_Body == nullptr) continue;
		//TODO: possible problem because globals and locals are stored separately so there could be 
		//a local and global with same id
		if (std::find(dirs.begin(), dirs.end(), currentObj.m_Direction)!=dirs.end())
		{
			return true;
		}
	}
	return false;

}
PhysicsBodyData* PhysicsBodyData::TryGetCollidingBody(const PhysicsBodyData& physicsBody)
{
	auto it = GetCollidingBodyIterator(physicsBody);
	if (it == m_collidingBodies.end()) return nullptr;
	return it->m_Body;
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

	for (const auto& currentObj : m_collidingBodies)
	{
		if (currentObj.m_Body== nullptr) continue;
		bodiesStr += std::format("{},", currentObj.m_Body->GetEntitySafe().ToString());
	}
	bodiesStr += "]";
	return bodiesStr;
}

std::string PhysicsBodyData::ToString() const
{
	return std::format("[PhysicsBody M:{}, G:{}, Vel:{} Accel:{}]", std::to_string(m_mass), 
		std::to_string(m_gravity), m_velocity.ToString(), m_acceleration.ToString());
}

PhysicsBodyData& PhysicsBodyData::Deserialize(const Json& json)
{
	m_mass = json.at("Mass").get<float>();
	m_gravity = json.at("Gravity").get<float>();
	m_profile.SetRestitution(json.at("Restitution").get<float>());
	m_profile.SetFriction(json.at("Friction").get<float>());

	SetVelocity(json.at("Velocity").get<Vec2>());
	m_terminalYVelocity= json.at("TerminalVelocity").get<float>();
	SetAcceleration(json.at("Acceleration").get<Vec2>());

	return *this;
}
Json PhysicsBodyData::Serialize(const PhysicsBodyData& component)
{
	return { {"Mass", m_mass}, {"Gravity", m_gravity}, {"Restitution", m_profile.GetRestitution()}, 
		{"Friction", m_profile.GetFriction()}, {"Velocity", m_velocity}, 
		{"TerminalVelocity", m_terminalYVelocity}, {"Acceleration", m_acceleration}};
}