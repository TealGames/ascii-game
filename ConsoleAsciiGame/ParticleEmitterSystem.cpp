#include "pch.hpp"
#include "ParticleEmitterSystem.hpp"
#include "Scene.hpp"
#include "CameraData.hpp"
#include "ParticleEmitterData.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 


namespace ECS
{
	ParticleEmitterSystem::ParticleEmitterSystem() {}

	void ParticleEmitterSystem::SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("AnimatorSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<ParticleEmitterData>(
			[this, &scene, &deltaTime](ParticleEmitterData& data, ECS::Entity& entity)-> void
			{
				auto renderLayers = scene.GetLayersMutable(data.m_renderLayers);

				//Assert(false, std::format("End index: {}", std::to_string(endIndex)));
				//size_t validParticleEndIndex = -1;
				if (!data.m_particles.IsEmpty())
				{
					//validParticleEndIndex = data.m_particles.size() - 1;
					//size_t i = 0;

					//TODO: it might be a good idea to further optimize this by not removing elements but using a pool

					//Note: there may be a better way of doing this, but we dont want to shift all elements (especially 
					//if there are many particles) every time we erase (and .remove only works using values not indices)
					
					//Note: we update pos, alive time and color then we push all particles to the end of the vector that are past their lifetime 
					//so they can all be removed in one go using erase
					data.m_particles.ExecuteOnAvailable([&](Particle& particle, size_t index)-> void
						{
							particle.m_Pos = particle.m_Pos + particle.m_Velocity * deltaTime;
							particle.m_AliveTime += deltaTime;
							particle.SetColorFromAliveTime(data.m_colorOverTime);

							//TODO: maybe try to opimize this better since if tehre are multiple particles to be removed
							//we have to shift the elements in the vector (especially if there are many elements)
							//each time separately
							if (particle.m_AliveTime >= particle.m_LifeTime)
							{
								data.m_particles.SetUnused(index);
								//data.m_activeParticles--;
							}
							else
							{
								AddParticleToLayers(data, particle, renderLayers);
								//data.m_activeParticles++;
							}
						});
				}

				const float fractionalParticlesToSpawn = data.m_spawnRate * deltaTime + data.m_lastFrameFractionParticles;
				const int wholeParticlesToSpawn = static_cast<int>(fractionalParticlesToSpawn);
				data.m_lastFrameFractionParticles = fractionalParticlesToSpawn - wholeParticlesToSpawn;

				Vec2 randomDir = {};
				Vec2 randomVel = {};
				float randomLifeTime = 0;
				const Color initialColor = data.m_colorOverTime.GetFirstColor(true);
				for (int i = 0; i < wholeParticlesToSpawn; i++)
				{
					randomDir = GenerateRandomDir();
					randomVel = GetVector(data.m_speedRange.GetRandom(), randomDir.GetAngle());
					randomLifeTime = data.m_lifetimeRange.GetRandom();

					Particle* particlePtr = data.m_particles.TryAdd(Particle(TextChar(initialColor, data.m_Char), data.m_FontData.m_FontSize,
						data.GetOriginWorldPos(), randomVel, randomLifeTime));
					AddParticleToLayers(data, *particlePtr, renderLayers);
				}

				data.m_activeParticles = data.m_particles.GetUsedSize();
			});
	}

	void ParticleEmitterSystem::AddParticleToLayers(const ParticleEmitterData& data, 
		const Particle& particle, std::vector<RenderLayer*>& renderLayers)
	{
		for (auto& layer : renderLayers)
		{
			layer->AddText(TextBufferPosition(particle.m_Pos, 
				particle.m_TextChar, data.m_FontData));
		}
	}
}