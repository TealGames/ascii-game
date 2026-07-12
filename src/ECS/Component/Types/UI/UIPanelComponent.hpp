#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/Color.hpp"
#include "Core/Primitives/Matrix.hpp"
//#include "raylib.h"
#include "Core/UI/UIRect.hpp"

namespace Engine::UI
{
	class UIRendererComponent;
	class UIPanelSystem;
	class UIPanelComponent : public ECS::Component
	{
	private:
		ColHDR4 m_color;
		UIRendererComponent* m_renderer;
	public:
		friend class UIPanelSystem;

	private:
	public:
		UIPanelComponent();
		UIPanelComponent(const ColHDR4 color);

		void SetColor(const ColHDR4 color);

		void Render(const float depth, const Mat3& globalModelMatrix);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


