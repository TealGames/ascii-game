#include "pch.hpp"
#include "PhysicsProfile.hpp"

namespace Physics
{
	PhysicsProfile::PhysicsProfile(const float& restitution, const float& friction)
		:m_RestitutionCoefficient(std::clamp(restitution, float(0), float(1))), 
		m_FrictionCoefficient(std::abs(friction))
	{

	}
}

