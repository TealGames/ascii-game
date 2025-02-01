#pragma once
#include "MultiBodySystem.hpp"
#include "ColliderOutlineBuffer.hpp"
#include "LineBuffer.hpp"

namespace ECS
{
	class PhysicsBodySystem : MultiBodySystem
	{
	private:
		ColliderOutlineBuffer m_colliderOutlineBuffer;
		LineBuffer m_lineBuffer;

	public:

	private:
	public:
		PhysicsBodySystem();
		void SystemUpdate(Scene& scene, const float& deltaTime) override;

		const ColliderOutlineBuffer& GetColliderBuffer() const;
		ColliderOutlineBuffer& GetColliderBufferMutable();

		const LineBuffer& GetLineBuffer() const;
		LineBuffer& GetLineBufferMutable();
	};
}


