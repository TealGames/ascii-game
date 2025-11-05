#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Model3d.hpp"

class Model3dAsset : public Asset
{
private:
	Rendering::Model3d m_model;
public:

private:
public:
	Model3dAsset(const std::filesystem::path& path);
	~Model3dAsset() = default;

	const Rendering::Model3d& GetModel() const;
	Rendering::Model3d& GetModelMutable();
	void UpdateAssetFromFile() override;
};

bool HasModel3dExtension(const std::string& extension);