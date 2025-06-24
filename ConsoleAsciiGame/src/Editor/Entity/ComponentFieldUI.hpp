#pragma once
#include <functional>
#include <string>
#include "ECS/Component/ComponentField.hpp"

class UIInteractionManager;
class ComponentUI;
class PopupUIManager;
namespace Input { class InputManager; }
class UITextComponent;
class UITransformData;
class UILayout;

class ComponentFieldUI //: public ITreeGUIConstructible
{
private:
	const Input::InputManager* m_inputManager;
	UITransformData* m_guiLayout;
	ComponentField* m_fieldInfo;
	std::vector<void*> m_fields;

	const ComponentUI* m_componentGUI;
	UITextComponent* m_fieldNameText;
public:

private:
	void SetupInputFields(UITransformData& fieldNameTextTransform, const bool fieldsStartNewLine);
	const Input::InputManager& GetInputManager() const;
	ComponentField& GetFieldInfo();

public:
	ComponentFieldUI(const Input::InputManager& inputManager, PopupUIManager& popupManager,
		const ComponentUI& componentGUI, UILayout& parent);
	ComponentFieldUI(const ComponentFieldUI&) = default;
	~ComponentFieldUI();

	//void Init();
	void SetField(ComponentField& field);
	void Update();
	void SetFieldToInternal();
	void SetInternalWithInput();
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
	//ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) override;

	const ComponentField& GetFieldInfo() const;

	const ComponentUI& GetComponentGUISafe() const;
};

