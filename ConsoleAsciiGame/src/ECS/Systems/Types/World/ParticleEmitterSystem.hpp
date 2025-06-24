#pragma once
#include "MultiBodySystem.hpp"
#include "RenderLayer.hpp"

class ParticleEmitterData;
class Particle;

namespace ECS
{
	class ParticleEmitterSystem : public MultiBodySystem
	{
	private:
	public:

	private:
		void AddParticleToLayers(const ParticleEmitterData& data, 
			const Particle& particle, std::vector<RenderLayer*>& renderLayers);

	public:
		ParticleEmitterSystem();

		void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) override;
	};

}

