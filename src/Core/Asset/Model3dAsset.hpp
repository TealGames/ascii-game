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

	/// <summary>
	/// Will write all vertices in the model to a VTX file
	/// format to the same path as the current asset.
	/// This is useful for reducing size of 3d model formats
	/// to a smaller custom-file type
	/// </summary>
	/// <returns></returns>
	void WriteModelAsCompressedFormat() const;
	void ReadModelAsCompressedFormat();
};

bool HasModel3dExtension(const std::string& extension);