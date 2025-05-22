#pragma once
#include "GUIElement.hpp"
#include <optional>

struct PopupGUI : public GUIElement
{
	PopupGUI();
	virtual ~PopupGUI() = default;

	RenderInfo Render(const RenderInfo& parentInfo) override;
	virtual void InsideRender(const RenderInfo& parentInfo) = 0;

	virtual void Update(const float deltaTime) = 0;
};

