#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIRect.hpp"

class TextureAsset;
class UIRendererData;
namespace ECS { class UITextureSystem; }
class UITextureData : public Component
{
private:
	const TextureAsset* m_texture;
	UIRendererData* m_renderer;
public:
	friend class ECS::UITextureSystem;

private:
public:
	UITextureData();
	UITextureData(const TextureAsset& texture);

	const TextureAsset* GetTexture() const;
	bool HasTexture() const;

	UIRect Render(const UIRect& parentInfo);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

