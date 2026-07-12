#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include <vector>
#include <cstdint>
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Primitives/ColorGradient.hpp"
#include "Utils/DataStructure/ObjectPool.hpp"

namespace Engine::ParticleSystem
{
	class Particle
	{
	private:
	public:
		Rendering::TextChar m_TextChar;
		WorldPosition3D m_Pos;
		Vec3 m_Velocity;
		Vec2 m_FontArea;
		/// <summary>
		/// The current duration in seconds this particle has been alive for
		/// </summary>
		float m_AliveTime;
		/// <summary>
		/// The normalized time in seconds for this particle when it will be destroyed
		/// </summary>
		float m_LifeTime;

	private:
	public:
		Particle(const Rendering::TextChar& text, const Vec2& fontArea,
			const WorldPosition3D& pos, const Vec3& vel, const float& lifeTime);

		void SetColorFromAliveTime(const ColHDRGradient& color);
	};

	class ParticleEmitterComponent : public ECS::Component
	{
	private:
	public:
		WorldPosition3D m_originTransformOffset;

		Rendering::WorldFontProperties m_FontData;
		//The color of any particle over the course of its lifetime
		ColHDRGradient m_lifetimeColor;
		char m_Char;
		Rendering::RenderLayerType m_renderLayers;

		//NOTE: for ranges, X is MIN, Y is MAX
		Vec2 m_speedRange;
		Vec2 m_lifetimeRange;
		float m_spawnRate;
		float m_normalizedTime;

		static const std::uint8_t MAX_PARTICLES = 255;
		::Utils::ObjectPool<Particle> m_particles;
		//Since low particle numbers and high fps may make it difficult to spawn
		//any particles, we bring them over from past frames to add up fractional amounts to whole
		float m_lastFrameFractionParticles;

		//The amount of particles active in the scene. NOTE: the amount of particles can usually be 
		//determined by (LifetimeRange.max- LifetimeRange.min)/2 + LifetimeRange.Min * spawnRate
		//Which makes sense because the avernage lifetime value will determine how many particles are left 
		//at any time which is changed based on how many are spawned in one second
		std::uint8_t m_activeParticles;

	private:
	public:
		ParticleEmitterComponent();
		ParticleEmitterComponent(const char& c, const Vec2& lifeTimeRange, const ColHDRGradient& colorOverTime,
			const Rendering::WorldFontProperties& fontData, const Rendering::RenderLayerType& renderLayers,
			const WorldPosition3D& transformOffset, const Vec2& speedRange, const float& spawnRate);

		WorldPosition3D GetOriginWorldPos() const;
		void SetSpawnRate(const float& value);
		/// <summary>
		/// Gives an approximation for active particles at any time
		/// </summary>
		/// <returns></returns>
		int ApproximateAverageParticles() const;
		int CalculateMaxParticles() const;

		//std::vector<std::string> GetDependencyFlags() const override;
		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}