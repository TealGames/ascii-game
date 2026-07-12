#include "pch.hpp"
#include "ECS/Systems/Types/World/TestSystem.hpp"
#include "ECS/Component/Types/World/TestComponent.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 

namespace Engine
{
	TestSystem::TestSystem() {}

	void TestSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& cam, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("TestSystem::SystemUpdate");
#endif 

	}
}