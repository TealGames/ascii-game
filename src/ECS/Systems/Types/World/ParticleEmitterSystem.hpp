#pragma once
#include "Core/Rendering/RenderLayer.hpp"

class ParticleEmitterData;
class Particle;
class Scene;
class CameraComponent;

namespace ECS
{
	class ParticleEmitterSystem
	{
	private:
	public:

	private:
		void AddParticleToLayers(const ParticleEmitterData& data, 
			const Particle& particle, std::vector<RenderLayer*>& renderLayers);

		Vec2 GenerateRandomDir() const;

	public:
		ParticleEmitterSystem();

		void SystemUpdate(Scene& scene, CameraComponent& mainCamera, const float& deltaTime);
	};

}

