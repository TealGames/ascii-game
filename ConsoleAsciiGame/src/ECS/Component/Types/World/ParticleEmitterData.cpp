#include "pch.hpp"
#include "ECS/Component/Types/World/ParticleEmitterData.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include <functional>
#include "Core/Serialization/JsonSerializers.hpp"

Particle::Particle(const TextChar& text, const float& fontSize, const WorldPosition& pos, const Vec2& vel, const float& lifeTime)
	: m_TextChar(text), m_FontSize(fontSize), m_Pos(pos), m_Velocity(vel), m_AliveTime(0), m_LifeTime(lifeTime) {}

void Particle::SetColorFromAliveTime(const ColorGradient& color)
{
	m_TextChar.m_Color = color.GetColorAt(m_AliveTime / m_LifeTime, true);
}

ParticleEmitterData::ParticleEmitterData() : ParticleEmitterData('A', FloatRange(0,1), 
	ColorGradient(WHITE), {}, RenderLayerType::Player, {}, FloatRange(1, 1), float(5)) {}

ParticleEmitterData::ParticleEmitterData(const char& c, const FloatRange& lifeTimeRange, const ColorGradient& colorOverTime, 
	const FontProperties& fontData, const RenderLayerType& renderLayers,
	const WorldPosition& transformOffset, const FloatRange& speedRange, const float& spawnRate)
	: Component(), 
	m_Char(c), m_normalizedTime(0), m_lifetimeColor(colorOverTime), 
	m_lifetimeRange(lifeTimeRange), 
	//m_particles(std::min(static_cast<int>(MAX_PARTICLES), static_cast<int>(std::ceil(spawnRate* m_lifetimeRange.m_Max)))),
	m_particles(CalculateMaxParticles()), m_activeParticles(0), m_lastFrameFractionParticles(0),
	m_originTransformOffset(transformOffset), m_FontData(fontData), //m_randomizeDirection(randomizeDir), 
	m_speedRange(speedRange), m_spawnRate(spawnRate), m_renderLayers(renderLayers)
{

}

WorldPosition ParticleEmitterData::GetOriginWorldPos() const
{
	return GetEntity().GetTransform().GetGlobalPos() + m_originTransformOffset;
}
void ParticleEmitterData::SetSpawnRate(const float& value)
{
	m_spawnRate = std::abs(value);
}
int ParticleEmitterData::ApproximateAverageParticles() const
{
	//Note: this approximation uses 2/3 of lifetime to give extra room, but actual value lies around half of lifetime * spawnrate
	float average= ((m_lifetimeRange.m_Max - m_lifetimeRange.m_Min) *5/6 + m_lifetimeRange.m_Min) * m_spawnRate;
	return static_cast<int>(std::ceilf(average));
}
int ParticleEmitterData::CalculateMaxParticles() const
{
	return std::min(ApproximateAverageParticles(), static_cast<int>(MAX_PARTICLES));
}

void ParticleEmitterData::InitFields()
{
	m_Fields = { ComponentField("ParticlesSpawned", &m_activeParticles, false), 
		ComponentField("SpawnRate", (std::function<void(float)>)[this](float newVal)-> void 
	{
		SetSpawnRate(newVal); 
	}, &m_spawnRate)};
}
//std::vector<std::string> ParticleEmitterData::GetDependencyFlags() const
//{
//	return {};
//}

std::string ParticleEmitterData::ToString() const
{
	return std::format("[ParticleEmitter char:{} lifetimeRange:{} speedRange:{} spawnRate:{}]", 
		Utils::ToString(m_Char), m_lifetimeRange.ToString(), m_speedRange.ToString(), std::to_string(m_spawnRate));
}

void ParticleEmitterData::Deserialize(const Json& json)
{
	m_Char = json.at("Char").get<char>();
	m_lifetimeRange = json.at("LifetimeRange").get<FloatRange>();
	m_lifetimeColor = json.at("LifetimeColor").get<ColorGradient>();
	m_FontData = json.at("FontData").get<FontProperties>();
	m_renderLayers = json.at("Layers").get<RenderLayerType>();
	m_originTransformOffset = json.at("Offset").get<WorldPosition>();
	m_speedRange = json.at("SpeedRange").get<FloatRange>();
	m_spawnRate = json.at("SpawnRate").get<float>();

	m_particles.TryReserveNewSize(CalculateMaxParticles());
}
Json ParticleEmitterData::Serialize()
{
	return { {"Char", m_Char}, {"LifetimeRange", m_lifetimeRange}, {"LifetimeColor", m_lifetimeColor}, {"FontData", m_FontData}, 
		{"Layers", m_renderLayers}, {"Offset", m_originTransformOffset}, {"SpeedRange", m_speedRange}, {"SpawnRate", m_spawnRate}};
}