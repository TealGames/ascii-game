#pragma once
#include "Unused/EntityMapper.hpp"
#include "Core/Scene/Scene.hpp"

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::ECS
{
	template<typename T>
	class SingleBodySystem
	{
	private:
	public:

	private:
	public:
		virtual void SystemUpdate(Scenes::Scene& scene, T& component, const float& deltaTime) = 0;
	};
}