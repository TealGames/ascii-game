#pragma once
#include <vector>
#include "EntityMapper.hpp"
#include "Scene.hpp"

namespace ECS
{
	class MultiBodySystem
	{
	private:
	public:

	private:
	public:
		virtual void SystemUpdate(Scene& scene, const float& deltaTime) = 0;
	};
}
