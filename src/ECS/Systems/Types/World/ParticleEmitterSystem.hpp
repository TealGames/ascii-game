#pragma once
#include "ECS/Systems/MultiBodySystem.hpp"
#include "Core/Rendering/RenderLayer.hpp"

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

		Vec2 GenerateRandomDir() const;

	public:
		ParticleEmitterSystem();

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime) override;
	};

}

