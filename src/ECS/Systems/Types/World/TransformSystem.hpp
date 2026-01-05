#pragma once
#include <vector>
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Point2DInt.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"
#include "Core/Scene/Scene.hpp"

namespace ECS
{
	class TransformSystem
	{
	private:
	public:

	private:
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
		//void UpdateLastFramePos(Scene& scene);
	};
}


