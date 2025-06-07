#pragma once
#include "TextBuffer.hpp"
#include "IRenderable.hpp"
#include "UIInputField.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

class SpriteCharGUI : public IRenderable
{
private:	
	UIInputField m_charField;

public:

private:
public:
	SpriteCharGUI(Input::InputManager& inputManager);

	void Update();
	RenderInfo Render(const RenderInfo& renderInfo) override;

	TextBufferCharPosition GetAsTextBufferChar() const;
};

