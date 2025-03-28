#include "pch.hpp"
#include "ParticleEmitterData.hpp"
#include "Entity.hpp"
#include <functional>

Particle::Particle(const TextChar& text, const float& fontSize, const WorldPosition& pos, const Vec2& vel, const float& lifeTime)
	: m_TextChar(text), m_FontSize(fontSize), m_Pos(pos), m_Velocity(vel), m_AliveTime(0), m_LifeTime(lifeTime) {}

void Particle::SetColorFromAliveTime(const ColorGradient& color)
{
	m_TextChar.m_Color = color.GetColorAt(m_AliveTime / m_LifeTime, true);
}

ParticleEmitterData::ParticleEmitterData(const char& c, const FloatRange& lifeTimeRange, const ColorGradient& colorOverTime, 
	const FontData& fontData, const RenderLayerType& renderLayers,
	const WorldPosition& transformOffset, const FloatRange& range, const float& spawnRate)
	: ComponentData(HighestDependecyLevel::None), 
	m_Char(c), m_normalizedTime(0), m_colorOverTime(colorOverTime), 
	m_lifetimeRange(lifeTimeRange), 
	//m_particles(std::min(static_cast<int>(MAX_PARTICLES), static_cast<int>(std::ceil(spawnRate* m_lifetimeRange.m_Max)))),
	m_particles(MAX_PARTICLES), m_activeParticles(0), m_lastFrameFractionParticles(0),
	m_originTransformOffset(transformOffset), m_FontData(fontData), //m_randomizeDirection(randomizeDir), 
	m_speedRange(range), m_spawnRate(spawnRate), m_renderLayers(renderLayers) 
{

}

WorldPosition ParticleEmitterData::GetOriginWorldPos() const
{
	return GetEntitySafe().m_Transform.m_Pos + m_originTransformOffset;
}
void ParticleEmitterData::SetSpawnRate(const float& value)
{
	m_spawnRate = std::abs(value);
}

void ParticleEmitterData::InitFields()
{
	//TODO: implement
	m_Fields = { ComponentField("ParticlesSpawned", &m_activeParticles, false), 
		ComponentField("SpawnRate", (std::function<void(float)>)[this](float newVal)-> void 
	{
		SetSpawnRate(newVal); 
	}, &m_spawnRate)};
}

std::string ParticleEmitterData::ToString() const
{
	//TODO: implement
	return "";
}

void ParticleEmitterData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json ParticleEmitterData::Serialize()
{
	//TODO: implement
	return {};
}