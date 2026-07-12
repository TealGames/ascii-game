#pragma once

namespace Engine::Physics
{
	class SurfacePhysicsProfile
	{
	private:
		/// <summary>
		/// How bouncy an object is where 0 means a perfectly inelastic collision (energy lost/no bounciness)
		/// and 1 means a perfectly elastic collision (mechanical energy conserved/max bounciness)
		/// </summary>
		float m_RestitutionCoefficient;
		float m_FrictionCoefficient;

	public:

	private:
		float GetBoundedRestitution(float input) const;
		float GetBoundedFriction(float input) const;
	public:
		SurfacePhysicsProfile();
		SurfacePhysicsProfile(float restitution, float friction);

		void SetRestitution(float restitution);
		void SetFriction(float friction);

		float GetRestitution() const;
		float& GetRestitutionMutable();

		float GetFriction() const;
		float& GetFrictionMutable();
	};
}