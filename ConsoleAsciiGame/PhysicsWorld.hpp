#pragma once
#include <vector>
#include "Point2DInt.hpp"
#include "PhysicsBodyData.hpp"

namespace Physics
{
	using BodyCollectionType = std::vector<PhysicsBodyData*>;
	class PhysicsWorld
	{
	private:
		//TODO: this needs to get optimized since this can get really slow
		//Probably optimize using a vector that holds bodies and a binary tree that holds aabb data that is used
		//to check intersection with progressivley smaller nodes until we reach leaf nodes which hold the actual objects
		
		/// <summary>
		/// All the bodies that will be used in the physics simulation
		/// </summary>
		BodyCollectionType m_bodies;
	public:

	private:
	public:
		PhysicsWorld();

		const BodyCollectionType& GetBodies() const;
		BodyCollectionType& GetBodiesMutable();

		void AddBody(PhysicsBodyData& body);

		void Update(const float& deltaTime);
	};

	bool DoBodiesIntersect(const PhysicsBodyData& body1, const PhysicsBodyData& body2);
}


