#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIRect.hpp"

namespace Engine::Rendering { class TextureAsset; }
namespace Engine::UI
{
	class UIRendererComponent;
	class UITextureSystem;
	class UITextureComponent : public ECS::Component
	{
	private:
		const Rendering::TextureAsset* m_texture;
		UIRendererComponent* m_renderer;
	public:
		friend class UITextureSystem;

	private:
	public:
		UITextureComponent();
		UITextureComponent(const Rendering::TextureAsset& texture);

		const Rendering::TextureAsset* GetTexture() const;
		bool HasTexture() const;

		UIRect Render(const UIRect& parentInfo);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}