#pragma once
#include "Core/Window/Window.hpp"

namespace Core
{
	Window CreateGlfwWindow(const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName, const UpdateCallbackType& updateCallback);
	void SetCurrentContextWindow(Window& window);
}
