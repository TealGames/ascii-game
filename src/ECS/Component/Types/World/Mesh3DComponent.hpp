#pragma once
#include "Core/Rendering/Model3d.hpp"
#include "ECS/Component/Component.hpp"

class Mesh3DComponent : public Component
{
private:
public:
	const Rendering::ModelMesh* m_Mesh;
	Rendering::Material* m_Material;

private:
public:
	Mesh3DComponent();
	Mesh3DComponent(Rendering::Model3d& model, const size_t meshIndex, 
		Rendering::Material* overrideMaterial = nullptr);

	void SetModel(Rendering::Model3d& model, const size_t meshIndex, 
		Rendering::Material* overrideMaterial = nullptr);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};