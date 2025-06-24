#pragma once
#include "PhysicsWorld.hpp"

namespace SceneManagement
{
	class SceneManager;
};
class CollisionRegistry;

namespace Physics
{
	class PhysicsManager
	{
	private:
		SceneManagement::SceneManager& m_sceneManager;
		Physics::PhysicsWorld m_physics;
	public:

	private:
	public:
		PhysicsManager(SceneManagement::SceneManager& sceneManager, CollisionRegistry& collisionRegistry);

		const Physics::PhysicsWorld& GetPhysicsWorld() const;
		Physics::PhysicsWorld& GetPhysicsWorldMutable();
	};
}
