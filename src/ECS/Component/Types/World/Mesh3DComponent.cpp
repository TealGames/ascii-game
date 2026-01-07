#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"

Mesh3DComponent::Mesh3DComponent() : Component(), m_Mesh(nullptr), m_Material(nullptr) {}
Mesh3DComponent::Mesh3DComponent(Rendering::Model3d& model, const size_t meshIndex, Rendering::Material* overrideMaterial)
	: Component(), m_Mesh(nullptr), m_Material(nullptr)
{
	SetModel(model, meshIndex, overrideMaterial);
}

void Mesh3DComponent::SetModel(Rendering::Model3d& model, const size_t objectIndex, 
	Rendering::Material* overrideMaterial)
{
	Rendering::ModelObject& obj = model.m_Objects[objectIndex];
	m_Mesh = &obj.m_Mesh;
	m_Material = overrideMaterial!= nullptr? overrideMaterial : &obj.m_Material;
}

void Mesh3DComponent::InitFields()
{
	m_Fields = {};
}
std::string Mesh3DComponent::ToString() const
{
	return std::format("[Mesh3d]");
}

void Mesh3DComponent::Deserialize(const Json& json)
{
	//TODO: implement
}
Json Mesh3DComponent::Serialize()
{
	//TODO: implement
	return {};
}