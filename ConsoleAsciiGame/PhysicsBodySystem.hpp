#pragma once
#include "MultiBodySystem.hpp"
#include "ColliderOutlineBuffer.hpp"

namespace ECS
{
	class PhysicsBodySystem : MultiBodySystem
	{
	private:
		ColliderOutlineBuffer m_ColliderOutlineBuffer;

	public:

	private:
	public:
		PhysicsBodySystem();
		void SystemUpdate(Scene& scene, const float& deltaTime) override;

		const ColliderOutlineBuffer* TryGetColliderBuffer() const;
		ColliderOutlineBuffer* TryGetColliderBufferMutable();
	};
}


