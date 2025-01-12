#pragma once
#include <vector>
#include "Component.hpp"
#include "Point2DInt.hpp"
#include "TransformData.hpp"
#include "MultiBodySystem.hpp"
#include "Scene.hpp"
#include "CartesianPosition.hpp"

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

		void SetPos(TransformData& data, const CartesianPosition& newPos);
		void SetPosX(TransformData& data, const int& newX);
		void SetPosY(TransformData& data, const int& newY);

		void SetPosDeltaX(TransformData& data, const int& xDelta);
		void SetPosDeltaY(TransformData& data, const int yDelta);
		void SetPosDelta(TransformData& data, const CartesianPosition& moveDelta);

		bool HasMovedThisFrame(TransformData& data) const;

		void SystemUpdate(Scene& scene, const float& deltaTime) override;
		void UpdateLastFramePos(Scene& scene);
	};
}


