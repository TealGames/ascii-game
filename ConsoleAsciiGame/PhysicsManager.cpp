#include "pch.hpp"
#include "PhysicsManager.hpp"
#include "SceneManager.hpp"

namespace Physics
{
	PhysicsManager::PhysicsManager(SceneManagement::SceneManager& sceneManager, CollisionRegistry& collisionRegistry)
		: m_sceneManager(sceneManager), m_physics(collisionRegistry)
	{
		m_sceneManager.m_OnSceneChange.AddListener([this](Scene* newScene) mutable -> void
			{
				if (newScene == nullptr) return;
				//Assert(false, std::format("Scene changed"));

				m_physics.ClearAllBodies();
				for (auto& entity : newScene->GetAllEntitiesMutable())
				{
					if (PhysicsBodyData* maybeBody = entity->TryGetComponentMutable<PhysicsBodyData>())
					{
						m_physics.AddBody(*maybeBody);
					}
				}
			});
	}

	const Physics::PhysicsWorld& PhysicsManager::GetPhysicsWorld() const
	{
		return m_physics;
	}

	Physics::PhysicsWorld& PhysicsManager::GetPhysicsWorldMutable()
	{
		return m_physics;
	}
}
