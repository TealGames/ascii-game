#pragma once
#include <vector>
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Point2DInt.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Scene/Scene.hpp"

namespace ECS
{
	class TransformSystem : public MultiBodySystem
	{
	private:
	public:

	private:
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
		void UpdateLastFramePos(Scene& scene);
	};
}


