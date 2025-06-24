#include "pch.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "HelperFunctions.hpp"
#include "EntityRendererData.hpp"
#include "SpriteAnimation.hpp"
#include "EntityData.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 
namespace ECS
{
	SpriteAnimatorSystem::SpriteAnimatorSystem(EntityRendererSystem& entityRenderer) 
		: m_EntityRenderer(entityRenderer) {}

	void SpriteAnimatorSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("SpriteAnimatorSystem::SystemUpdate");
#endif 

		if (deltaTime <= 0) return;

		scene.OperateOnComponents<SpriteAnimatorData>(
			[this, &scene, deltaTime](SpriteAnimatorData& data)-> void
			{
				if (!data.IsPlayingAnimation()) return;

				SpriteAnimation* currentAnim = data.TryGetPlayingAnimationMutable();
				if (currentAnim == nullptr) return;

				if (!Assert(this, 0 <= currentAnim->m_FrameIndex && currentAnim->m_FrameIndex < currentAnim->m_Frames.size(),
					std::format("Tried to update frame idnex in sprite animator, but frame index:{} is at out of bound position of frames:[0, {})",
						std::to_string(currentAnim->m_FrameIndex), std::to_string(currentAnim->m_Frames.size()))))
					return;

				if (currentAnim->m_Frames.empty()) return;

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

	void SpriteAnimatorSystem::SetVisual(EntityData& entity, const SpriteAnimation& animation) const
	{
		//TODO: this should maybe be included as dependency for the animator?
		EntityRendererData* renderer = entity.TryGetComponentMutable<EntityRendererData>();
		if (!Assert(this, renderer != nullptr, std::format("Tried to set the visual on sprite animator for entity: {} "
			"but it does not have entity renderer component", entity.m_Name))) return;

		const VisualData* currAnimVisual = animation.TryGetCurrentVisualData();
		if (!Assert(this, currAnimVisual != nullptr, std::format("Tried to set the animation visual for entity:{} "
			"but failed to retrieve current animation visual. FrameIndex:{}", entity.m_Name, std::to_string(animation.m_FrameIndex))))
			return;

		//TODO: perhaps there should be some optimization here and maybe we can reintroduce frame deltas in some way?
		//however it may be defiiculty becase pviots can change and then the positions may too
		renderer->OverrideVisualData(*currAnimVisual);

		/*
		//Note: since the first animation (or any animation) may not have the same size as the default visual of 
		//the renderer we need to take that into account in order to be able to use the frame delta optimization
		//and without having any weird out of bounds bugs
		if (renderer->GetVisualSize() != currAnimVisual->GetBufferSize()) renderer->OverrideVisualData(*currAnimVisual);
		else
		{
			const SpriteAnimationDelta* animDelta = animation.TryGetCurrentAnimationDelta();
			if (!Assert(this, animDelta != nullptr, std::format("Tried to set the animation visual for entity:{} "
				"but failed to retrieve current animation delta. FrameIndex:{}", entity.GetName(), std::to_string(animation.m_FrameIndex))))
				return;

			renderer->SetVisualDataDeltas(animDelta->m_VisualDelta);

		}
		renderer->m_MutatedThisFrame = true;
		*/
	}
}