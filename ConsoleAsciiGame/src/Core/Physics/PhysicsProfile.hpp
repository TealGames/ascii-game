#pragma once

namespace Physics
{
	class PhysicsProfile
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
		float GetBoundedRestitution(const float& input) const;
		float GetBoundedFriction(const float& input) const;
	public:
		PhysicsProfile();
		PhysicsProfile(const float& restitution, const float& friction);

		void SetRestitution(const float& restitution);
		void SetFriction(const float& friction);

		const float& GetRestitution() const;
		float& GetRestitutionMutable();

		const float& GetFriction() const;
		float& GetFrictionMutable();
	};
}