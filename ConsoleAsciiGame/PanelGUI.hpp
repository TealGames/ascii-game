#pragma once
#include "GUIElement.hpp"
#include "Color.hpp"
#include "raylib.h"

class PanelGUI : public GUIElement
{
private:
	Color m_color;
public:

private:
public:
	PanelGUI();
	PanelGUI(const Color color);

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
};

