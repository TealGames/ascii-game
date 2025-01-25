#include "pch.hpp"
#include "TransformSystem.hpp"
#include "Point2DInt.hpp"
#include "TransformData.hpp"
#include "CartesianPosition.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
	void TransformSystem::SystemUpdate(Scene& scene, const float& deltaTime) {}

	void TransformSystem::UpdateLastFramePos(Scene& scene)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TransformSystem::UpdateLastFramePos");
#endif 

		scene.OperateOnComponents<TransformData>(
			[](TransformData& transform, ECS::Entity& entity)->void
			{
				transform.m_LastFramePos = transform.m_Pos;
			});
	}

	//TODO: do scene bound checking

}

