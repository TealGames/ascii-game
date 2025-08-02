#pragma once

#ifdef GLFW
#include "Core/Window/Window.hpp"

namespace Core
{
	namespace Glfw
	{
		Window CreateWindow(const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName,
			const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback);
		void SetCurrentContextWindow(Window& window);
	}
}
#endif