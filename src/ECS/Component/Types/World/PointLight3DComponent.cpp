#include "ECS/Component/Types/World/PointLight3DComponent.hpp"

PointLight3DComponent::PointLight3DComponent() : PointLight3DComponent(0, {}) {}
PointLight3DComponent::PointLight3DComponent(const float radius, const Utils::Color color) :
	m_Radius(radius), m_Color(color) {}


void PointLight3DComponent::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json PointLight3DComponent::Serialize()
{
	//TODO: implement
	return {};
}

void PointLight3DComponent::InitFields()
{
	m_Fields = {};
}
std::string PointLight3DComponent::ToString() const
{
	return std::format("[PointLight3D]");
}