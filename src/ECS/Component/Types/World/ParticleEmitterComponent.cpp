#include <functional>
#include "pch.hpp"
#include "ECS/Component/Types/World/ParticleEmitterComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace ThisNamespace = Engine::ParticleSystem;
namespace Engine::ParticleSystem
{
	Particle::Particle(const Rendering::TextChar& text, const Vec2& fontArea, const WorldPosition3D& pos, const Vec3& vel, const float& lifeTime)
		: m_TextChar(text), m_FontArea(fontArea), m_Pos(pos), m_Velocity(vel), m_AliveTime(0), m_LifeTime(lifeTime) {}

	void Particle::SetColorFromAliveTime(const ColHDRGradient& color)
	{
		m_TextChar.m_Color = color.GetColorAt(m_AliveTime / m_LifeTime, true);
	}

	ParticleEmitterComponent::ParticleEmitterComponent() : ParticleEmitterComponent('A', Vec2(0, 1),
		ColorGradient(COLOR_WHITE), {}, Rendering::RenderLayerType::Player, {}, Vec2(1, 1), float(5)) {}

	ParticleEmitterComponent::ParticleEmitterComponent(const char& c, const Vec2& lifeTimeRange, const ColHDRGradient& colorOverTime,
		const Rendering::WorldFontProperties& fontData, const Rendering::RenderLayerType& renderLayers,
		const WorldPosition3D& transformOffset, const Vec2& speedRange, const float& spawnRate)
		: Component(),
		m_Char(c), m_normalizedTime(0), m_lifetimeColor(colorOverTime),
		m_lifetimeRange(lifeTimeRange),
		//m_particles(std::min(static_cast<int>(MAX_PARTICLES), static_cast<int>(std::ceil(spawnRate* m_lifetimeRange.m_Max)))),
		m_particles(CalculateMaxParticles()), m_activeParticles(0), m_lastFrameFractionParticles(0),
		m_originTransformOffset(transformOffset), m_FontData(fontData), //m_randomizeDirection(randomizeDir), 
		m_speedRange(speedRange), m_spawnRate(spawnRate), m_renderLayers(renderLayers)
	{
		//LogWarning(std::format("Created particle with capacity:{}", m_particles.GetMaxCapacity()));
		Assert(m_particles.GetMaxCapacity() != 0, "Attempted to create particle emiiter with char:{} "
			"but max particle approximation for reserving particle pool size was: {}", c, m_particles.GetMaxCapacity());
	}

	WorldPosition3D ParticleEmitterComponent::GetOriginWorldPos() const
	{
		return GetEntity().GetTransform().GetWorldPos() + m_originTransformOffset;
	}
	void ParticleEmitterComponent::SetSpawnRate(const float& value)
	{
		m_spawnRate = std::abs(value);
	}
	int ParticleEmitterComponent::ApproximateAverageParticles() const
	{
		//Note: this approximation uses 2/3 of lifetime to give extra room, but actual value lies around half of lifetime * spawnrate
		float average = ((m_lifetimeRange.m_Y - m_lifetimeRange.m_X) * 5 / 6 + m_lifetimeRange.m_X) * m_spawnRate;
		return static_cast<int>(std::ceilf(average));
	}
	int ParticleEmitterComponent::CalculateMaxParticles() const
	{
		return std::min(ApproximateAverageParticles(), static_cast<int>(MAX_PARTICLES));
	}

	void ParticleEmitterComponent::InitFields()
	{
		m_Fields = { ECS::ComponentField("ParticlesSpawned", &m_activeParticles, false),
			ECS::ComponentField("SpawnRate", (std::function<void(float)>)[this](float newVal)-> void
		{
			SetSpawnRate(newVal);
		}, &m_spawnRate) };
	}

	void ParticleEmitterComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("Char", m_Char);
		serializer.AddProperty("LifetimeRange", m_lifetimeRange);
		serializer.AddProperty("LifetimeColor", m_lifetimeColor);
		serializer.AddProperty("FontData", m_FontData);
		serializer.AddProperty("Layers", m_renderLayers);
		serializer.AddProperty("Offset", m_originTransformOffset);
		serializer.AddProperty("SpeedRange", m_speedRange);
		serializer.AddProperty("SpawnRate", m_spawnRate);
	}
	void ParticleEmitterComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("Char", &m_Char);
		deserializer.GetProperty("LifetimeRange", &m_lifetimeRange);
		deserializer.GetProperty("LifetimeColor", &m_lifetimeColor);
		deserializer.GetProperty("FontData", &m_FontData);
		deserializer.GetProperty("Layers", &m_renderLayers);
		deserializer.GetProperty("Offset", &m_originTransformOffset);
		deserializer.GetProperty("SpeedRange", &m_speedRange);
		deserializer.GetProperty("SpawnRate", &m_spawnRate);

		m_particles.TryReserveNewSize(CalculateMaxParticles());
	}

	std::string ParticleEmitterComponent::ToString() const
	{
		return std::format("[ParticleEmitter char:{} lifetimeRange:{} speedRange:{} spawnRate:{}]",
			::Utils::ToString(m_Char), m_lifetimeRange.ToString(), m_speedRange.ToString(), std::to_string(m_spawnRate));
	}
}
