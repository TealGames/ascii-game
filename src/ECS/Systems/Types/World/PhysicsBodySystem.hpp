#pragma once
#include "Core/Physics/PhysicsManager.hpp"

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::Physics
{
	class PhysicsManager;
	class PhysicsBodySystem
	{
	private:
		PhysicsManager& m_physicsManager;

	public:

	private:
	public:
		PhysicsBodySystem(Physics::PhysicsManager& physicsManager);
		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}


