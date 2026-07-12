#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Core/Asset/Model3dAsset.hpp"
#include "Core/Asset/MaterialAsset.hpp"

namespace Engine::Rendering
{
	Mesh3DComponent::Mesh3DComponent() : Component(), m_ModelAsset(nullptr), m_ObjectIndex(0), m_MaterialAsset(nullptr) {}
	Mesh3DComponent::Mesh3DComponent(Rendering::Model3dAsset& model, const size_t meshIndex, Rendering::MaterialAsset* overrideMaterial)
		: Component(), m_ModelAsset(nullptr), m_ObjectIndex(0)
	{
		SetModel(model, meshIndex, overrideMaterial);
	}

	void Mesh3DComponent::SetModel(Rendering::Model3dAsset& modelAsset, const size_t objectIndex,
		Rendering::MaterialAsset* overrideMaterial)
	{
		Rendering::Model3d& model = modelAsset.GetModelMutable();
		ENGINE_ASSERT(objectIndex < model.m_Objects.size(), "Tried to set Mesh3D model with out of bounds index");

		m_ModelAsset = &modelAsset;
		m_ObjectIndex = objectIndex;
		Rendering::ModelObject& modelObj = model.m_Objects[m_ObjectIndex];
		m_MaterialAsset = overrideMaterial != nullptr ? overrideMaterial : modelObj.m_MaterialAsset;
	}

	const Rendering::ModelObject* Mesh3DComponent::GetModelObject() const
	{
		if (m_ModelAsset == nullptr)
			return nullptr;

		return &m_ModelAsset->GetModel().m_Objects[m_ObjectIndex];
	}
	const Rendering::ModelMesh* Mesh3DComponent::GetMesh() const
	{
		const Rendering::ModelObject* modelObj = GetModelObject();
		return &modelObj->m_Mesh;
	}

	void Mesh3DComponent::InitFields()
	{
		m_Fields = {};
	}
	void Mesh3DComponent::Serialize(Serialization::Serializer& serializer) const
	{
		;
		serializer.AddProperty("Model", Serialization::TrySerializeOptionalAsset(m_ModelAsset));
		serializer.AddProperty("Material", Serialization::TrySerializeOptionalAsset(m_MaterialAsset));
		serializer.AddProperty("ObjIdx", m_ObjectIndex);
	}
	void Mesh3DComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		std::optional<Serialization::SerializedAsset> maybeSerializedAsset = std::nullopt;
		deserializer.GetProperty("Model", &maybeSerializedAsset);
		m_ModelAsset = Serialization::TryDeserializeOptionalTypeAsset<Model3dAsset>(maybeSerializedAsset);

		deserializer.GetProperty("Material", &maybeSerializedAsset);
		m_MaterialAsset = Serialization::TryDeserializeOptionalTypeAsset<MaterialAsset>(maybeSerializedAsset);

		deserializer.GetProperty("ObjIdx", &m_ObjectIndex);
	}

	std::string Mesh3DComponent::ToString() const
	{
		return std::format("[Mesh3d]");
	}
}