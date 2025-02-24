#pragma once
#include "RenderInfo.hpp"

struct IRenderable
{
	virtual ScreenPosition Render(const RenderInfo& renderInfo) = 0;
	virtual ~IRenderable() = default;
};