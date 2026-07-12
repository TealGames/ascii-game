#include "pch.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine
{
	void TransformSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime) 
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TransformSystem::SystemUpdate");
#endif 
	}
}

