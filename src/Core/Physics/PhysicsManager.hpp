#pragma once
#include "Core/Physics/PhysicsWorld.hpp"

namespace Engine::Scenes { class SceneManager; }
namespace Engine::Physics
{
	using SceneManager = Engine::Scenes::SceneManager;

	class CollisionRegistry;
	class PhysicsManager
	{
	private:
		SceneManager& m_sceneManager;
		PhysicsWorld m_physics;
	public:

	private:
	public:
		PhysicsManager(SceneManager& sceneManager, CollisionRegistry& collisionRegistry);

		const PhysicsWorld& GetPhysicsWorld() const;
		PhysicsWorld& GetPhysicsWorldMutable();
	};
}
