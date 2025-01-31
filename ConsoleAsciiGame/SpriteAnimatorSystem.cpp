#include "pch.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "HelperFunctions.hpp"
#include "EntityRendererData.hpp"
#include "ProfilerTimer.hpp"

namespace ECS
{
	SpriteAnimatorSystem::SpriteAnimatorSystem(EntityRendererSystem& entityRenderer) 
		: m_EntityRenderer(entityRenderer) {}

	void SpriteAnimatorSystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("SpriteAnimatorSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<SpriteAnimatorData>(
			[this, &scene, &deltaTime](SpriteAnimatorData& data, ECS::Entity& entity)-> void
			{
				if (data.m_NormalizedTime >= data.m_EndTime && !data.m_Loop) return;

				data.m_NormalizedTime += deltaTime;
				if (data.m_NormalizedTime > data.m_VisualDeltas[data.m_VisualDeltaIndex].m_Time && data.m_Loop)
				{
					data.m_VisualDeltaIndex = (data.m_VisualDeltaIndex + 1) % data.m_VisualDeltas.size();
					if (data.m_VisualDeltaIndex==0) data.m_NormalizedTime -= data.m_EndTime;
					SetVisual(entity, data.m_VisualDeltas[data.m_VisualDeltaIndex].m_VisualDelta);
				}
			});
	}

	void SpriteAnimatorSystem::SetVisual(ECS::Entity& entity, const VisualDataPositions& positions)
	{
		//TODO: this should maybe be abstracted into the system?
		EntityRendererData* renderer = entity.TryGetComponent<EntityRendererData>();
		if (!Utils::Assert(this, renderer != nullptr, std::format("Tried to set the visual on sprite animator for entity: {} "
			"but it does not have entity renderer component", entity.m_Name))) return;

		renderer->SetVisualData(positions);
		renderer->m_MutatedThisFrame = true;
	}
}