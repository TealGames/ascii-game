#pragma once
#include "MultiBodySystem.hpp"
#include "ColliderOutlineBuffer.hpp"
#include "LineBuffer.hpp"
#include "PhysicsManager.hpp"

namespace ECS
{
	class PhysicsBodySystem : MultiBodySystem
	{
	private:
		ColliderOutlineBuffer m_colliderOutlineBuffer;
		LineBuffer m_lineBuffer;
		Physics::PhysicsManager& m_physicsManager;

	public:

	private:
	public:
		PhysicsBodySystem(Physics::PhysicsManager& physicsManager);
		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;

		const ColliderOutlineBuffer& GetColliderBuffer() const;
		ColliderOutlineBuffer& GetColliderBufferMutable();

		const LineBuffer& GetLineBuffer() const;
		LineBuffer& GetLineBufferMutable();
	};
}


