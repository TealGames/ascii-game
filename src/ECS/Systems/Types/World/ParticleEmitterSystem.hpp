#pragma once

namespace Engine::Rendering { class RenderLayer; }
namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::ParticleSystem
{
	class Particle;
	class ParticleEmitterComponent;
	class ParticleEmitterSystem
	{
	private:
	public:

	private:
		void AddParticleToLayers(const ParticleEmitterComponent& data, 
			const Particle& particle, std::vector<Rendering::RenderLayer*>& renderLayers);

		Vec2 GenerateRandomDir() const;

	public:
		ParticleEmitterSystem();

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};

}

