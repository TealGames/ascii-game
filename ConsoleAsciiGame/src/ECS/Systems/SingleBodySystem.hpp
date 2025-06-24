#pragma once
#include "Unused/EntityMapper.hpp"
#include "Core/Scene/Scene.hpp"

namespace ECS
{
	template<typename T>
	class SingleBodySystem
	{
	private:
	public:

	private:
	public:
		virtual void SystemUpdate(Scene& scene, T& component, const float& deltaTime) = 0;
	};
}