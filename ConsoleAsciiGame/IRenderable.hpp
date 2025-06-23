#pragma once
#include "UIRect.hpp"

struct IRenderable
{
	virtual ~IRenderable() = default;
	virtual UIRect Render(const UIRect& renderInfo) = 0;
};