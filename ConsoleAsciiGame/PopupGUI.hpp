#pragma once
#include "IRenderable.hpp"
#include <optional>

struct PopupGUI : public IRenderable
{
	virtual ~PopupGUI() = default;
	virtual ScreenPosition Render(const RenderInfo& renderInfo) = 0;
	virtual void Update(const float deltaTime) = 0;
};

