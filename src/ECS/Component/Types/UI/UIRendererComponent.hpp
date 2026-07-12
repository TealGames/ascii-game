#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIRect.hpp"

namespace Engine::Rendering { class Renderer; }
namespace Engine::UI
{
	class UIRenderSystem;
	class UIRendererComponent : public ECS::Component
	{
	private:
		Rendering::Renderer* m_renderer;
		UIRect m_lastRenderArea;
	public:
		friend class UIRenderSystem;

	private:
	public:
		UIRendererComponent();

		const UIRect& GetLastRenderRect() const;
		Rendering::Renderer& GetRendererMutable();

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


