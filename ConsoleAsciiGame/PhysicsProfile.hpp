#pragma once

namespace Physics
{
	struct PhysicsProfile
	{
		/// <summary>
		/// How bouncy an object is where 0 means a perfectly inelastic collision (no energy lost/no bounciness)
		/// and 1 means a perfectly elastic collision (mechanical energy conserved/max bounciness)
		/// </summary>
		float m_RestitutionCoefficient;
		float m_FrictionCoefficient;

		PhysicsProfile(const float& restitution, const float& friction);
	};
}