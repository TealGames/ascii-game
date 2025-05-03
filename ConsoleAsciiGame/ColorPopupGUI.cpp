#include "pch.hpp"
#include "ColorPopupGUI.hpp"
#include "Vec2.hpp"
#include "raylib.h"

static constexpr float PICKER_SPACE_AREA_FACTOR = 0.7;

ColorPopupGUI::ColorPopupGUI(GUISelectorManager& guiSelector, const Input::InputManager& input) 
	: m_rField(input, guiSelector, InputFieldType::Integer, InputFieldFlag::None, GUISettings()), m_rText(), m_slider(guiSelector, input, { 0, 1 }, GUISettings()) {}

void ColorPopupGUI::Init()
{
	m_slider.Init();
}

ScreenPosition ColorPopupGUI::Render(const RenderInfo& renderInfo)
{
	const Vec2 pickerSpace = Vec2(renderInfo.m_RenderSize.m_X, PICKER_SPACE_AREA_FACTOR * renderInfo.m_RenderSize.m_Y);
	const Vec2 displaySpace = Vec2(renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y) - pickerSpace;

	m_slider.Render(renderInfo);

	return renderInfo.m_RenderSize;
}
void ColorPopupGUI::Update(const float deltaTime)
{
	m_slider.Update();
}
