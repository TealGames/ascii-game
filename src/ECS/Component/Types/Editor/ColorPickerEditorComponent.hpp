#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Core/Primitives/Color.hpp"
#include <functional>

namespace Engine::Input { class InputManager; }
namespace Engine::UI
{
	class PopupUIManager;
	class UIPanelComponent;
	class UISelectableComponent;
	class UIRendererComponent;
}
namespace Engine::Editor::UI
{
	namespace UI = Engine::UI;
	using ColorPickerAction = std::function<void(ColHDR4)>;

	class UIColorPickerSystem;
	class ColorPickerEditorComponent : public ECS::Component
	{
	private:
		UI::PopupUIManager* m_popupManager;
		UI::UIRendererComponent* m_renderer;
		UI::UIPanelComponent* m_fieldPanel;
		UI::UISelectableComponent* m_selectable;
		//GUIStyle m_settings;
		ColHDR4 m_color;

		ColorPickerAction m_valueSetCallback;
	public:
		friend class UIColorPickerSystem;

	private:
	public:
		ColorPickerEditorComponent();
		ColorPickerEditorComponent(const UI::UIStyle& settings);

		void Init();

		void SetColor(const ColHDR4 color);
		//void SetSettings(const GUIStyle& settings);
		void SetValueSetAction(const ColorPickerAction& action);

		ColHDR4 GetColor() const;
		const UI::UIPanelComponent* GetFieldPanel() const;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


