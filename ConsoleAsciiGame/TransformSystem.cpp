#include "pch.hpp"
#include "TransformSystem.hpp"
#include "Point2DInt.hpp"
#include "TransformData.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	void TransformSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) {}

	void TransformSystem::UpdateLastFramePos(Scene& scene)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TransformSystem::UpdateLastFramePos");
#endif 

		scene.OperateOnComponents<TransformData>(
			[](TransformData& transform, ECS::Entity& entity)->void
			{
				transform.SetLocalPosLastFrame(transform.GetLocalPos());
			});
	}

	//TODO: do scene bound checking

}

