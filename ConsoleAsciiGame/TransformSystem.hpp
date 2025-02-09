#pragma once
#include <vector>
#include "Component.hpp"
#include "Point2DInt.hpp"
#include "TransformData.hpp"
#include "MultiBodySystem.hpp"
#include "Scene.hpp"

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

		void SystemUpdate(Scene& scene, const float& deltaTime) override;
		void UpdateLastFramePos(Scene& scene);
	};
}


