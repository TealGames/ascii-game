#include "pch.hpp"
#include "ECS/Component/Types/UI/UITextureData.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/GameRenderer.hpp"

UITextureData::UITextureData() : m_texture(nullptr), m_renderer(nullptr) {}
UITextureData::UITextureData(const TextureAsset& texture) : m_texture(&texture), m_renderer(nullptr) {}

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
	//LogError(std::format("Rendering texture for entity:{} has:{}", GetEntity().m_Name, m_texture!=nullptr));
	if (!HasTexture()) return {};

	const Vec2 globalScale = GetEntity().GetTransform().GetGlobalScale();
	const Vec2 renderAreaSize = renderRect.GetSize();
	const float minFitToAreaScale = std::min((float)renderAreaSize.m_X / m_texture->GetTexture().width, (float)renderAreaSize.m_Y / m_texture->GetTexture().height);
	const Vec2 scale = Vec2(minFitToAreaScale * globalScale.m_X, minFitToAreaScale * globalScale.m_Y);
	
	//LogWarning(std::format("Scale of ui texture:{} global scale:{} local scale:{}", scale.ToString(), globalScale.ToString(), GetEntity().GetTransform().GetLocalScale().ToString()));
	m_renderer->GetRendererMutable().AddTextureCall(renderRect.m_TopLeftPos, m_texture->GetTexture(), 0, scale, WHITE);
	return renderRect;
}

void UITextureData::InitFields()
{
	m_Fields = {};
}
std::string UITextureData::ToString() const
{
	return std::format("[UITexture tex:{}]", HasTexture()? m_texture->ToString() : "None");
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