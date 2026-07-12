#include "pch.hpp"
#include "Core/Physics/SurfacePhysicsProfile.hpp"

namespace Engine::Physics
{
	SurfacePhysicsProfile::SurfacePhysicsProfile() : SurfacePhysicsProfile(0, 0) {}

	SurfacePhysicsProfile::SurfacePhysicsProfile(float restitution, float friction)
		:m_RestitutionCoefficient(GetBoundedRestitution(restitution)),
		m_FrictionCoefficient(GetBoundedFriction(friction))
	{

	}

	float SurfacePhysicsProfile::GetBoundedRestitution(float restitution) const
	{
		return std::clamp(restitution, float(0), float(1));
	}
	float SurfacePhysicsProfile::GetBoundedFriction(float friction) const
	{
		return std::abs(friction);
	}

	void SurfacePhysicsProfile::SetRestitution(float restitution)
	{
		m_RestitutionCoefficient = GetBoundedRestitution(restitution);
	}
	void SurfacePhysicsProfile::SetFriction(float friction)
	{
		m_FrictionCoefficient = GetBoundedFriction(friction);
	}

	float SurfacePhysicsProfile::GetRestitution() const
	{
		return m_RestitutionCoefficient;
	}
	float& SurfacePhysicsProfile::GetRestitutionMutable()
	{
		return m_RestitutionCoefficient;
	}

	float SurfacePhysicsProfile::GetFriction() const
	{
		return m_FrictionCoefficient;
	}
	float& SurfacePhysicsProfile::GetFrictionMutable()
	{
		return m_FrictionCoefficient;
	}
}

