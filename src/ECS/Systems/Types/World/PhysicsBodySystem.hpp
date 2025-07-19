#pragma once
#include "ECS/Systems/MultiBodySystem.hpp"
//#include "Core/Collision/ColliderOutlineBuffer.hpp"
#include "Core/Rendering/LineBuffer.hpp"
#include "Core/Physics/PhysicsManager.hpp"

namespace ECS
{
	class PhysicsBodySystem : MultiBodySystem
	{
	private:
		//ColliderOutlineBuffer m_colliderOutlineBuffer;
		LineBuffer m_lineBuffer;
		Physics::PhysicsManager& m_physicsManager;

	public:

	private:
	public:
		PhysicsBodySystem(Physics::PhysicsManager& physicsManager);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;

		//const ColliderOutlineBuffer& GetColliderBuffer() const;
		//ColliderOutlineBuffer& GetColliderBufferMutable();

		const LineBuffer& GetLineBuffer() const;
		LineBuffer& GetLineBufferMutable();
	};
}


