#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include <vector>
#include <cstdint>
#include "Utils/Math/WorldPosition.hpp"
#include "Utils/Math/FloatRange.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Utils/Data/ColorGradient.hpp"
#include "Utils/DataStructure/ObjectPool.hpp"

struct Particle
{
	TextChar m_TextChar;
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

	Particle(const TextChar& text, const Vec2& fontArea, 
		const WorldPosition3D& pos, const Vec3& vel, const float& lifeTime);

	void SetColorFromAliveTime(const ColorGradient& color);
};

struct ParticleEmitterData : Component
{
	WorldPosition3D m_originTransformOffset;

	WorldFontProperties m_FontData;
	//The color of any particle over the course of its lifetime
	ColorGradient m_lifetimeColor;
	char m_Char;
	RenderLayerType m_renderLayers;

	//bool m_randomizeDirection;
	FloatRange m_speedRange;
	FloatRange m_lifetimeRange;
	float m_spawnRate;
	float m_normalizedTime;

	static const std::uint8_t MAX_PARTICLES = 255;
	ObjectPool<Particle> m_particles;
	//Since low particle numbers and high fps may make it difficult to spawn
	//any particles, we bring them over from past frames to add up fractional amounts to whole
	float m_lastFrameFractionParticles;

	//The amount of particles active in the scene. NOTE: the amount of particles can usually be 
	//determined by (LifetimeRange.max- LifetimeRange.min)/2 + LifetimeRange.Min * spawnRate
	//Which makes sense because the avernage lifetime value will determine how many particles are left 
	//at any time which is changed based on how many are spawned in one second
	std::uint8_t m_activeParticles;

	ParticleEmitterData();
	ParticleEmitterData(const char& c, const FloatRange& lifeTimeRange, const ColorGradient& colorOverTime, 
		const WorldFontProperties& fontData, const RenderLayerType& renderLayers, 
		const WorldPosition3D& transformOffset, const FloatRange& speedRange, const float& spawnRate);

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

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

