#pragma once
#include "RenderInfo.hpp"

struct IRenderable
{
	virtual ~IRenderable() = default;
	virtual RenderInfo Render(const RenderInfo& renderInfo) = 0;
};