#include "pch.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "Utils/Data/Point2DInt.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
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
			[](TransformData& transform)->void
			{
				transform.SetLocalPosLastFrame(transform.GetLocalPos());
			});
	}

	//TODO: do scene bound checking

}

