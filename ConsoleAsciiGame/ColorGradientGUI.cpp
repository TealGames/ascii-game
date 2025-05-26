#include "pch.hpp"
#include "ColorGradientGUI.hpp"
#include "raylib.h"

ColorGradientGUI::ColorGradientGUI(const ColorGradient& gradient) : m_gradient(gradient) {}

void ColorGradientGUI::SetGradient(const ColorGradient& gradient)
{
	m_gradient = gradient;
}
const ColorGradient& ColorGradientGUI::GetGradient() const
{
	return m_gradient;
}

void ColorGradientGUI::Update(const float deltaTime)
{
}

RenderInfo ColorGradientGUI::ElementRender(const RenderInfo& renderInfo)
{
	//TODO: finish implementing
	Color startColor = m_gradient.GetFirstColor(false);
	Color endColor = m_gradient.GetLastColor(false);
	DrawRectangleGradientH(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_X, 
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, startColor, endColor);

	return {};
}