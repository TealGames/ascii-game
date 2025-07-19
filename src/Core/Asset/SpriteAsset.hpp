#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Visual/VisualData.hpp"

class SpriteAsset : public Asset
{
private:
	VisualData m_visual;
public:
	static const std::string EXTENSION;

protected:
public:
	SpriteAsset(const std::filesystem::path& path);

	const VisualData& GetVisual() const;
	VisualData& GetVisualMutable();

	void UpdateAssetFromFile() override;
	void SaveToPath(const std::filesystem::path& path) override;

	std::string ToString() const override;
};
