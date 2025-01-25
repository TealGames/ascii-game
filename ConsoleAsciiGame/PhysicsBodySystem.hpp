#pragma once
#include "MultiBodySystem.hpp"

namespace ECS
{
	class PhysicsBodySystem : MultiBodySystem
	{
	private:
	public:

	private:
	public:
		PhysicsBodySystem();
		void SystemUpdate(Scene& scene, const float& deltaTime) override;
	};
}


