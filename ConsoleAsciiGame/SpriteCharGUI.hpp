#pragma once
#include "TextBuffer.hpp"
#include "IRenderable.hpp"
#include "InputField.hpp"

class GUISelectorManager;
namespace Input { class InputManager; }

class SpriteCharGUI : public IRenderable
{
private:	
	InputField m_charField;

public:

private:
public:
	SpriteCharGUI(GUISelectorManager& guiSelector, Input::InputManager& inputManager);

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	TextBufferCharPosition GetAsTextBufferChar() const;
};

