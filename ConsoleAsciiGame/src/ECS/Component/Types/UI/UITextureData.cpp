#include "pch.hpp"
#include "ECS/Component/Types/UI/UITextureData.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/GameRenderer.hpp"

UITextureData::UITextureData() : m_texture(nullptr), m_renderer(nullptr) {}
UITextureData::UITextureData(const TextureAsset& texture) : m_texture(&texture) {}

const TextureAsset* UITextureData::GetTexture() const
{
	return m_texture;
}
bool UITextureData::HasTexture() const
{
	return m_texture != nullptr;
}
UIRect UITextureData::Render(const UIRect& renderRect)
{
	if (m_texture == nullptr) return {};

	const Vec2Int renderAreaSize = renderRect.GetSize();
	const float scaleX = renderAreaSize.m_X / m_texture->GetTexture().width;
	const float scaleY = renderAreaSize.m_Y / m_texture->GetTexture().height;
	m_renderer->GetRendererMutable().AddTextureCall(renderRect.m_TopLeftPos, m_texture->GetTexture(), 0, std::min(scaleX, scaleY), WHITE);
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