#pragma once
#include "TextBuffer.hpp"
#include "UIInputField.hpp"

class UIInteractionManager;
namespace Input { class InputManager; }

class SpriteCharUI //: public IRenderable
{
private:	
	UIInputField m_charField;

public:

private:
public:
	SpriteCharUI(Input::InputManager& inputManager);

	void Update();
	//RenderInfo Render(const RenderInfo& renderInfo) override;

	TextBufferCharPosition GetAsTextBufferChar() const;
};

