#pragma once
#include "ComponentData.hpp"
#include "GUIRect.hpp"

class TextureAsset;
class UITextureData : public Component
{
private:
	const TextureAsset* m_texture;
public:

private:
public:
	UITextureData();
	UITextureData(const TextureAsset& texture);

	const TextureAsset* GetTexture() const;
	bool HasTexture() const;

	GUIRect Render(const GUIRect& parentInfo);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

