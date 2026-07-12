#include "pch.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Core/Scene/Scene.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Systems/Types/World/SpriteAnimatorSystem.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorComponent.hpp"
#include "Core/Visual/SpriteAnimation.hpp"

#ifdef ENABLE_PROFILER
#include "Core/Analyzation/ProfilerTimer.hpp"
#endif 
namespace Engine::Animation
{
	SpriteAnimatorSystem::SpriteAnimatorSystem(Rendering::EntityRenderer2DSystem& entityRenderer) 
		: m_EntityRenderer(entityRenderer) {}

	void SpriteAnimatorSystem::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("SpriteAnimatorSystem::SystemUpdate");
#endif 

		if (deltaTime <= 0) return;

		scene.OperateOnActiveComponents<SpriteAnimatorComponent>(
			[this, &scene, deltaTime](SpriteAnimatorComponent& data)-> void
			{
				if (!data.IsPlayingAnimation()) 
					return;

				SpriteAnimation* currentAnim = data.TryGetPlayingAnimationMutable();
				if (currentAnim == nullptr) 
					return;

				if (!Assert(0 <= currentAnim->m_FrameIndex && currentAnim->m_FrameIndex < currentAnim->m_Frames.size(),
					"Tried to update frame idnex in sprite animator, but frame index:{} is at out of bound position of frames:[0, {})",
					std::to_string(currentAnim->m_FrameIndex), std::to_string(currentAnim->m_Frames.size())))
					return;

				if (currentAnim->m_Frames.empty()) 
					return;

				const float animDeltaTime = deltaTime* currentAnim->m_AnimationSpeed;
				if (currentAnim->m_NormalizedTime >= currentAnim->m_SingleLoopLength && !currentAnim->m_Loop)
					return;

				currentAnim->m_NormalizedTime += animDeltaTime;
				if (currentAnim->m_NormalizedTime > currentAnim->m_Frames[currentAnim->m_FrameIndex].m_Time && currentAnim->m_Loop)
				{
					currentAnim->m_FrameIndex = (currentAnim->m_FrameIndex + 1) % currentAnim->m_Frames.size();
					if (currentAnim->m_FrameIndex==0) currentAnim->m_NormalizedTime -= currentAnim->m_SingleLoopLength;
					SetVisual(data.GetEntityMutable(), *currentAnim);
				}
			});
	}

	void SpriteAnimatorSystem::SetVisual(ECS::EntityData& entity, const SpriteAnimation& animation) const
	{
		//TODO: this should maybe be included as dependency for the animator?
		Rendering::EntityRenderer2DComponent* renderer = entity.TryGetComponentMutable<Rendering::EntityRenderer2DComponent>();
		if (!Assert(renderer != nullptr, "Tried to set the visual on sprite animator for entity: {} "
			"but it does not have entity renderer component", entity.m_Name)) 
			return;

		const Rendering::VisualData* currAnimVisual = animation.TryGetCurrentVisualData();
		if (!Assert(currAnimVisual != nullptr, "Tried to set the animation visual for entity:{} "
			"but failed to retrieve current animation visual. FrameIndex:{}", entity.m_Name, std::to_string(animation.m_FrameIndex)))
			return;

		//TODO: perhaps there should be some optimization here and maybe we can reintroduce frame deltas in some way?
		//however it may be defiiculty becase pviots can change and then the positions may too
		renderer->OverrideVisualData(*currAnimVisual);
	}
}