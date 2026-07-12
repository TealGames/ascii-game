#include "pch.hpp"
#include "ECS/Component/Types/UI/UITextureComponent.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UITextureComponent::UITextureComponent() : m_texture(nullptr), m_renderer(nullptr) {}
	UITextureComponent::UITextureComponent(const Rendering::TextureAsset& texture) : m_texture(&texture), m_renderer(nullptr) {}

	const Rendering::TextureAsset* UITextureComponent::GetTexture() const
	{
		return m_texture;
	}
	bool UITextureComponent::HasTexture() const
	{
		return m_texture != nullptr;
	}
	UIRect UITextureComponent::Render(const UIRect& renderRect)
	{
		//LogError(std::format("Rendering texture for entity:{} has:{}", GetEntity().m_Name, m_texture!=nullptr));
		if (!HasTexture()) return {};

		const Vec3 globalScale = GetEntity().GetTransform().GetWorldScale();
		const Vec2 renderAreaSize = renderRect.GetSize().AsVec2();
		const float minFitToAreaScale = std::min((float)renderAreaSize.m_X / m_texture->GetTexture().GetInfo().m_TexelSize.m_X,
			(float)renderAreaSize.m_Y / m_texture->GetTexture().GetInfo().m_TexelSize.m_Y);
		const Vec2 scale = Vec2(minFitToAreaScale * globalScale.m_X, minFitToAreaScale * globalScale.m_Y);

		//LogWarning(std::format("Scale of ui texture:{} global scale:{} local scale:{}", scale.ToString(), globalScale.ToString(), GetEntity().GetTransform().GetLocalScale().ToString()));

		//TODO: add ui render call
		//m_renderer->GetRendererMutable().AddTextureCall(renderRect.m_TopLeftPos, m_texture->GetTexture(), 0, scale, Color_WHITE);
		return renderRect;
	}

	void UITextureComponent::InitFields()
	{
		m_Fields = {};
	}
	void UITextureComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UITextureComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UITextureComponent::ToString() const
	{
		return std::format("[UITexture tex:{}]", HasTexture() ? m_texture->ToString() : "None");
	}
}
