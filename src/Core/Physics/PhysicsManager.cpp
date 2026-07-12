#include "pch.hpp"
#include "Core/Physics/PhysicsManager.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::Physics
{
	PhysicsManager::PhysicsManager(SceneManager& sceneManager, CollisionRegistry& collisionRegistry)
		: m_sceneManager(sceneManager), m_physics(collisionRegistry)
	{
		m_sceneManager.m_OnActiveSceneChange.AddListener([this](Engine::Scenes::Scene* newScene) mutable -> void
			{
				if (newScene == nullptr) return;
				//Assert(false, std::format("Scene changed"));

				m_physics.ClearAllBodies();
				for (auto& entity : newScene->GetAllEntitiesMutable())
				{
					if (PhysicsBodyComponent* maybeBody = entity->TryGetComponentMutable<PhysicsBodyComponent>())
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
