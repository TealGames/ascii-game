#include "pch.hpp"
#include "UIColorGradient.hpp"
#include "raylib.h"

UIColorGradient::UIColorGradient(const ColorGradient& gradient) : m_gradient(gradient) {}

void UIColorGradient::SetGradient(const ColorGradient& gradient)
{
	m_gradient = gradient;
}
const ColorGradient& UIColorGradient::GetGradient() const
{
	return m_gradient;
}

void UIColorGradient::Update(const float deltaTime)
{
}

RenderInfo UIColorGradient::ElementRender(const RenderInfo& renderInfo)
{
	//TODO: finish implementing
	Color startColor = m_gradient.GetFirstColor(false);
	Color endColor = m_gradient.GetLastColor(false);
	DrawRectangleGradientH(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_X, 
		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, startColor, endColor);

	return {};
}