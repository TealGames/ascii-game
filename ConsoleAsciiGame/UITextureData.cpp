#include "pch.hpp"
#include "UITextureData.hpp"
#include "TextureAsset.hpp"
#include "EntityData.hpp"
#include "UIRendererComponent.hpp"
#include "GameRenderer.hpp"

UITextureData::UITextureData() : m_texture(nullptr) {}
UITextureData::UITextureData(const TextureAsset& texture) : m_texture(&texture) {}

const TextureAsset* UITextureData::GetTexture() const
{
	return m_texture;
}
bool UITextureData::HasTexture() const
{
	return m_texture != nullptr;
}
GUIRect UITextureData::Render(const GUIRect& renderRect)
{
	if (m_texture == nullptr) return {};
	UIRendererData* renderer = GetEntitySafeMutable().TryGetComponentMutable<UIRendererData>();

	const Vec2Int renderAreaSize = renderRect.GetSize();
	const float scaleX = renderAreaSize.m_X / m_texture->GetTexture().width;
	const float scaleY = renderAreaSize.m_Y / m_texture->GetTexture().height;
	renderer->GetRendererMutable().AddTextureCall(renderRect.m_TopLeftPos, m_texture->GetTexture(), 0, std::min(scaleX, scaleY), WHITE);
	return renderRect;
}

void UITextureData::InitFields()
{
	m_Fields = {};
}
std::string UITextureData::ToString() const
{
	return std::format("[UITexture tex:{}]", m_texture!=nullptr? m_texture->ToString() : "None");
}

void UITextureData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UITextureData::Serialize()
{
	//TODO: implement
	return {};
}