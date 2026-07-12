#pragma once
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/Primitives/ColorGradient.hpp"

namespace Engine::UI { class UIPanelComponent; }
namespace Engine::Editor::UI
{
	class ColorGradientUISettings
	{
	private:
	public:
		ColorChannel m_DiplayChannels;
		bool m_DoHDR;
	};

	class ColorGradientEditorComponent : public Engine::UI::UISelectableComponent
	{
	private:
		ColHDRGradient m_gradient;
	public:

	private:
	public:
		ColorGradientEditorComponent();
		ColorGradientEditorComponent(const ColHDRGradient& gradient);

		void SetGradient(const ColHDRGradient& gradient);
		const ColHDRGradient& GetGradient() const;

		void Update(const float deltaTime);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
	};
}

