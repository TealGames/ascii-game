#pragma once
#include "GUIRect.hpp"

struct IRenderable
{
	virtual ~IRenderable() = default;
	virtual GUIRect Render(const GUIRect& renderInfo) = 0;
};