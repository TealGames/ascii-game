#include <limits>
#include <optional>
#include "pch.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "Utils/Debug.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Physics
{
	MoveContraints::MoveContraints(const bool constrainX, const bool constrainY)
		: m_ConstrainX(constrainX), m_ConstrainY(constrainY) {}

	PhysicsBodyComponent::PhysicsBodyComponent(const CollisionBoxComponent* coliisionBox, const float mass,
		const float gravity, const float terminalYVelocity)
		: Component(),
		m_mass(0.0f), //m_aabb(CreateAABB(boundingBoxSize, transformOffset)),
		//m_collider(GetEntitySafe().m_Transform, boundingBoxSize, transformOffset),
		//m_collider(transform, boundingBoxSize, transformOffset),
		m_collider(coliisionBox),
		m_velocity(), m_acceleration(),
		m_gravity(-std::abs(gravity)), m_terminalYVelocity(-std::abs(terminalYVelocity)),
		m_profile(1, 1), m_physicsSimulation(nullptr), m_contraints(), m_isGrounded(false)
	{
		SetMass(mass);
	}

	PhysicsBodyComponent::PhysicsBodyComponent() :
		PhysicsBodyComponent(nullptr, 0, 0, std::numeric_limits<float>::max()) {}

	PhysicsBodyComponent::PhysicsBodyComponent(const CollisionBoxComponent* collisionBox, const float mass) :
		PhysicsBodyComponent(collisionBox, mass, 0, 0) {}

	void PhysicsBodyComponent::InitFields()
	{
		m_Fields =
		{
			ECS::ComponentField("Mass(KG)", &m_mass), ECS::ComponentField("Gravity", &m_gravity),
			ECS::ComponentField("TerminalYVelocity", &m_terminalYVelocity),
			ECS::ComponentField("Velocity(m/s)",(std::function<void(Vec2)>)[this](Vec2 vec)-> void {SetVelocity(vec); }, &m_velocity),
			ECS::ComponentField("Accel(m/s2)", (std::function<void(Vec2)>)[this](Vec2 vec)-> void {SetAcceleration(vec); }, &m_acceleration),
			ECS::ComponentField("Restitution", (std::function<void(float)>)[this](float restitution)-> void
							{m_profile.SetRestitution(restitution); }, &(m_profile.GetRestitutionMutable())),
			ECS::ComponentField("Friction", (std::function<void(float)>)[this](float friction)-> void
							{m_profile.SetFriction(friction); },& (m_profile.GetFrictionMutable())),
		};
	}

	void PhysicsBodyComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("m", m_mass);
		serializer.AddProperty("g", m_gravity);
		serializer.AddProperty("e", m_profile.GetRestitution());
		serializer.AddProperty("u", m_profile.GetFriction());
		serializer.AddProperty("v", m_velocity);
		serializer.AddProperty("vt", m_terminalYVelocity);
		serializer.AddProperty("a", m_acceleration);
	}
	void PhysicsBodyComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		float mass = 0.0f;
		deserializer.GetProperty("m", &mass);
		SetMass(mass);

		deserializer.GetProperty("g", &m_gravity);

		float restitution = 0.0f;
		float friction = 0.0f;
		deserializer.GetProperty("e", &restitution);
		deserializer.GetProperty("u", &friction);
		m_profile.SetRestitution(restitution);
		m_profile.SetFriction(friction);

		deserializer.GetProperty("v", &m_velocity);
		deserializer.GetProperty("vt", &m_terminalYVelocity);
		deserializer.GetProperty("a", &m_acceleration);
	}

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
		SetVelocityDelta({ 0, yDelta });
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

	const Vec2& PhysicsBodyComponent::GetVelocity() const { return m_velocity; }
	const Vec2& PhysicsBodyComponent::GetAcceleration() const { return m_acceleration; }
	float PhysicsBodyComponent::GetGravity() const { return m_gravity; }

	bool PhysicsBodyComponent::IsExperiencingGravity() const
	{
		//TODO: this should probably consider if the object is groudned or not rather than a
		//accelerations since it can have net force a not equal to g, but since affected by gravity
		return ::Math::ApproximateEqualsF(m_acceleration.m_Y, m_gravity);
	}

	void PhysicsBodyComponent::SetIsGrounded(const bool grounded)
	{
		m_isGrounded = grounded;
	}
	bool PhysicsBodyComponent::IsGrounded() const { return m_isGrounded; }

	void PhysicsBodyComponent::SetMass(float mass)
	{
		m_mass = std::abs(mass);
	}
	float PhysicsBodyComponent::GetMass() const { return m_mass; }
	bool PhysicsBodyComponent::HasMass() const { return m_mass > 0; }
	Vec2 PhysicsBodyComponent::GetMomentum() const { return m_velocity * m_mass; }
	const Physics::SurfacePhysicsProfile& PhysicsBodyComponent::GetSurfacePhysicsProfile() const
	{
		return m_profile;
	}

	void PhysicsBodyComponent::SetConstraint(const MoveContraints& constraint)
	{
		m_contraints = constraint;
	}
	MoveContraints PhysicsBodyComponent::GetConstraint() const { return m_contraints; }
	bool PhysicsBodyComponent::HasXConstraint() const { return m_contraints.m_ConstrainX; }
	bool PhysicsBodyComponent::HasYConstraint() const { return m_contraints.m_ConstrainY; }
	bool PhysicsBodyComponent::HasAnyConstraints() const { return HasXConstraint() || HasYConstraint(); }

	const CollisionBoxComponent& PhysicsBodyComponent::GetCollisionBox() const
	{
		ENGINE_ASSERT(m_collider != nullptr, "Tried to get collider data for entity:{} but was NULL", GetEntity().m_Name);
		return *m_collider;
	}

	std::string PhysicsBodyComponent::ToString() const
	{
		return std::format("[PhysicsBody collider:{} M:{}, G:{}, Vel:{} Accel:{}]",
			GetCollisionBox().ToString(), std::to_string(m_mass),
			std::to_string(m_gravity), m_velocity.ToString(), m_acceleration.ToString());
	}
}
