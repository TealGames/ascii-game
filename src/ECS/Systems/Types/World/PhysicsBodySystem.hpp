#pragma once
//#include "Core/Collision/ColliderOutlineBuffer.hpp"
#include "Core/Rendering/LineBuffer.hpp"
#include "Core/Physics/PhysicsManager.hpp"

class Scene;
class CameraComponent;
namespace ECS
{
	class PhysicsBodySystem
	{
	private:
		//ColliderOutlineBuffer m_colliderOutlineBuffer;
		//LineBuffer m_lineBuffer;
		Physics::PhysicsManager& m_physicsManager;

	public:

	private:
	public:
		PhysicsBodySystem(Physics::PhysicsManager& physicsManager);
		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);

		//const ColliderOutlineBuffer& GetColliderBuffer() const;
		//ColliderOutlineBuffer& GetColliderBufferMutable();

		/*const LineBuffer& GetLineBuffer() const;
		LineBuffer& GetLineBufferMutable();*/
	};
}


