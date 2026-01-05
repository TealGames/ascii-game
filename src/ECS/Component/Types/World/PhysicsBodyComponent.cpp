#include "pch.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"
#include <limits>
#include <optional>
#include "Core/Serialization/JsonSerializers.hpp"
#include "Utils/Debug.hpp"

MoveContraints::MoveContraints(const bool constrainX, const bool constrainY)
	: m_ConstrainX(constrainX), m_ConstrainY(constrainY) {}

PhysicsBodyComponent::PhysicsBodyComponent(const CollisionBoxData* coliisionBox, const float mass, 
	const float gravity, const float terminalYVelocity)
	: Component(),
	m_mass(std::abs(mass)), //m_aabb(CreateAABB(boundingBoxSize, transformOffset)),
	//m_collider(GetEntitySafe().m_Transform, boundingBoxSize, transformOffset),
	//m_collider(transform, boundingBoxSize, transformOffset),
	m_collider(coliisionBox),
	m_velocity(), m_acceleration(),
	m_gravity(-std::abs(gravity)), m_terminalYVelocity(-std::abs(terminalYVelocity)),
	m_profile(1, 1), m_physicsSimulation(nullptr), m_contraints(), m_isGrounded(false)
{
	//LogWarning(std::format("Created physics body of size: {} offset: {} that has min: {} max: {} size: {}",
	//boundingBoxSize.ToString(), transformOffset.ToString(), m_AABB.m_MinPos.ToString(), 
	//m_AABB.m_MaxPos.ToString(), m_AABB.GetSize().ToString());

	//ValidateAABB(m_aabb);
}

PhysicsBodyComponent::PhysicsBodyComponent() : 
	PhysicsBodyComponent(nullptr, 0, 0, std::numeric_limits<float>::max()) {}

PhysicsBodyComponent::PhysicsBodyComponent(const Json& json) : PhysicsBodyComponent()
{
	Deserialize(json);
}

PhysicsBodyComponent::PhysicsBodyComponent(const CollisionBoxData* collisionBox, const float mass) :
	PhysicsBodyComponent(collisionBox, mass, 0, 0) {}
	
void PhysicsBodyComponent::InitFields()
{
	m_Fields = 
	{	
		ComponentField("Mass(KG)", &m_mass), ComponentField("Gravity", &m_gravity),
		ComponentField("TerminalYVelocity", &m_terminalYVelocity), 
		ComponentField("Velocity(m/s)", [this](Vec2 vec)-> void {SetVelocity(vec);}, &m_velocity),
		ComponentField("Accel(m/s2)", [this](Vec2 vec)-> void {SetAcceleration(vec); }, &m_acceleration),
		ComponentField("Restitution", (std::function<void(float)>)[this](float restitution)-> void 
						{m_profile.SetRestitution(restitution); }, &(m_profile.GetRestitutionMutable())),
		ComponentField("Friction", (std::function<void(float)>)[this](float friction)-> void 
						{m_profile.SetFriction(friction); }, &(m_profile.GetFrictionMutable())),
	};
}

//bool PhysicsBodyData::ValidateAABB(const Physics::AABB& bounding) const
//{
//	const Vec2 size = bounding.GetSize();
//	if (Assert(size.m_X!=0 && size.m_Y!=0, 
//		std::format("Tried to create a Physics Body but the AABB cannot have 0 x or y size: {}. "
//			"This could be due to bad bounding size or offset!", size.ToString()))) 
//		return false;
//
//	return true;
//}
//
//Physics::AABB PhysicsBodyData::CreateAABB(const Vec2& boundingBoxSize, const WorldPosition& transformOffset)
//{
//	return {transformOffset- (boundingBoxSize/2), transformOffset+ (boundingBoxSize / 2) };
//}

void PhysicsBodyComponent::SetPhysicsWorldRef(const Physics::PhysicsWorld& world)
{
	m_physicsSimulation = &world;
}
void PhysicsBodyComponent::RemovePhysicsWorldRef()
{
	m_physicsSimulation = nullptr;
}
const Physics::PhysicsWorld& PhysicsBodyComponent::GetPhysicsWorldSafe()
{
	ENGINE_ASSERT(m_physicsSimulation != nullptr,
		"Tried to get physics world of body: '{}' but it is NULL", GetEntity().m_Name);

	return *m_physicsSimulation;
}
	
void PhysicsBodyComponent::SetVelocityXDelta(const float& xDelta)
{
	SetVelocityDelta({ xDelta, 0 });
}
void PhysicsBodyComponent::SetVelocityYDelta(const float& yDelta)
{
	SetVelocityDelta({0, yDelta});
}
void PhysicsBodyComponent::SetVelocityDelta(const Vec2& vel)
{
	SetVelocity(m_velocity + vel);
}
void PhysicsBodyComponent::SetVelocity(const Vec2& vel)
{
	//if (GetEntitySafe().GetName() == "player" && vel == Vec2::ZERO) Assert(false, "ZERO PLAYER VEL");
	m_velocity.m_X = vel.m_X;
	m_velocity.m_Y = std::max(vel.m_Y, m_terminalYVelocity);
}

void PhysicsBodyComponent::SetAcceleration(const Vec2& acc)
{
	m_acceleration = acc;
}

const Vec2& PhysicsBodyComponent::GetVelocity() const
{
	return m_velocity;
}

const Vec2& PhysicsBodyComponent::GetAcceleration() const
{
	return m_acceleration;
}

const float& PhysicsBodyComponent::GetGravity() const
{
	return m_gravity;
}

bool PhysicsBodyComponent::IsExperiencingGravity() const
{
	//TODO: this should probably consider if the object is groudned or not rather than a
	//accelerations since it can have net force a not equal to g, but since affected by gravity
	return Utils::ApproximateEqualsF(m_acceleration.m_Y, m_gravity);
}

void PhysicsBodyComponent::SetIsGrounded(const bool grounded)
{
	m_isGrounded = grounded;
}
bool PhysicsBodyComponent::IsGrounded() const
{
	return m_isGrounded;
}

const float& PhysicsBodyComponent::GetMass() const
{
	return m_mass;
}

bool PhysicsBodyComponent::HasMass() const
{
	return m_mass > 0;
}
Vec2 PhysicsBodyComponent::GetMomentum() const
{
	return m_velocity * m_mass;
}
const Physics::PhysicsProfile& PhysicsBodyComponent::GetPhysicsProfile() const
{
	return m_profile;
}

void PhysicsBodyComponent::SetConstraint(const MoveContraints& constraint)
{
	m_contraints = constraint;
}
MoveContraints PhysicsBodyComponent::GetConstraint() const
{
	return m_contraints;
}
bool PhysicsBodyComponent::HasXConstraint() const
{
	return m_contraints.m_ConstrainX;
}
bool PhysicsBodyComponent::HasYConstraint() const
{
	return m_contraints.m_ConstrainY;
}
bool PhysicsBodyComponent::HasAnyConstraints() const
{
	return HasXConstraint() || HasYConstraint();
}

const CollisionBoxData& PhysicsBodyComponent::GetCollisionBox() const
{
	ENGINE_ASSERT(m_collider != nullptr, "Tried to get collider data for entity:{} but was NULL", GetEntity().m_Name);
	return *m_collider;
}

std::string PhysicsBodyComponent::ToString() const
{
	/*return std::format("[PhysicsBody AABB:{} offset:{} M:{}, G:{}, Vel:{} Accel:{}]", 
		m_aabb.ToString(GetAABBCenterWorldPos()), m_transformOffset.ToString(), std::to_string(m_mass),
		std::to_string(m_gravity), m_velocity.ToString(), m_acceleration.ToString());*/

	//LogError("PHysics to string");
	return std::format("[PhysicsBody collider:{} M:{}, G:{}, Vel:{} Accel:{}]",
		GetCollisionBox().ToString(), std::to_string(m_mass),
		std::to_string(m_gravity), m_velocity.ToString(), m_acceleration.ToString());
}

void PhysicsBodyComponent::Deserialize(const Json& json)
{
	//TODO: add deserialize for transform offset and aabb

	//m_aabb= json.at("AABB").get<Physics::AABB>();
	//ValidateAABB(m_aabb);
	//m_collider= TryDeserializeComponent<CollisionBoxData>(json.at("Collider"), GetEntityMutable());
	//LogError(std::format("Finsihed deserialiation of collier"));
	//m_transformOffset = json.at("Offset").get<Vec2>();

	m_mass = json.at("Mass").get<float>();
	m_gravity = json.at("Gravity").get<float>();
	m_profile.SetRestitution(json.at("Restitution").get<float>());
	m_profile.SetFriction(json.at("Friction").get<float>());

	SetVelocity(json.at("Velocity").get<Vec2>());
	m_terminalYVelocity= json.at("TerminalVelocity").get<float>();
	SetAcceleration(json.at("Acceleration").get<Vec2>());
}
Json PhysicsBodyComponent::Serialize()
{
	return //{ {"AABB", m_aabb}, 
		//{ {"Offset", m_transformOffset}, 
		{ 
		//{"Collider", TrySerializeComponent<CollisionBoxData>(m_collider)},
		{ "Mass", m_mass }, {"Gravity", m_gravity}, {"Restitution", m_profile.GetRestitution()},
		{"Friction", m_profile.GetFriction()}, {"Velocity", m_velocity}, 
		{"TerminalVelocity", m_terminalYVelocity}, {"Acceleration", m_acceleration}};
}