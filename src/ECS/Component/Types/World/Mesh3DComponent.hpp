#pragma once
#include "ECS/Component/Component.hpp"

namespace Engine::Rendering
{
	class Model3dAsset;
	class MaterialAsset;
	class ModelObject;
	class ModelMesh;
	class Mesh3DComponent : public ECS::Component
	{
	private:
	public:
		Model3dAsset* m_ModelAsset;
		size_t m_ObjectIndex;
		MaterialAsset* m_MaterialAsset;

	private:
	public:
		Mesh3DComponent();
		Mesh3DComponent(Model3dAsset& model, const size_t meshIndex,
			MaterialAsset* overrideMaterial = nullptr);

		void SetModel(Model3dAsset& model, const size_t meshIndex,
			MaterialAsset* overrideMaterial = nullptr);

		const ModelObject* GetModelObject() const;
		const ModelMesh* GetMesh() const;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}
