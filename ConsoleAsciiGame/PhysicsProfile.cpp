#include "pch.hpp"
#include "PhysicsProfile.hpp"

namespace Physics
{
	PhysicsProfile::PhysicsProfile() : PhysicsProfile(0, 0) {}

	PhysicsProfile::PhysicsProfile(const float& restitution, const float& friction)
		:m_RestitutionCoefficient(GetBoundedRestitution(restitution)),
		m_FrictionCoefficient(GetBoundedFriction(friction))
	{

	}

	float PhysicsProfile::GetBoundedRestitution(const float& restitution) const
	{
		return std::clamp(restitution, float(0), float(1));
	}
	float PhysicsProfile::GetBoundedFriction(const float& friction) const
	{
		return std::abs(friction);
	}

	void PhysicsProfile::SetRestitution(const float& restitution)
	{
		m_RestitutionCoefficient = GetBoundedRestitution(restitution);
	}
	void PhysicsProfile::SetFriction(const float& friction)
	{
		m_FrictionCoefficient = GetBoundedFriction(friction);
	}

	const float& PhysicsProfile::GetRestitution() const
	{
		return m_RestitutionCoefficient;
	}
	float& PhysicsProfile::GetRestitutionMutable()
	{
		return m_RestitutionCoefficient;
	}

	const float& PhysicsProfile::GetFriction() const
	{
		return m_FrictionCoefficient;
	}
	float& PhysicsProfile::GetFrictionMutable()
	{
		return m_FrictionCoefficient;
	}
}

