#pragma once
#include "Asset.hpp"
#include "SpriteAnimation.hpp"

class SpriteAnimationAsset : public Asset
{
private:
	SpriteAnimation m_animation;
public:
	static const std::string EXTENSION;

protected:
public:
	SpriteAnimationAsset(const std::filesystem::path& path);

	const SpriteAnimation& GetAnimation() const;
	SpriteAnimation& GetAnimationMutable();

	void UpdateAssetFromFile() override;
	void SaveToPath(const std::filesystem::path& path) override;

	std::string ToString() const override;
};

