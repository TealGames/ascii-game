#pragma once

#ifdef GLFW
#include "Core/Window/Window.hpp"

namespace Engine::Core::Glfw
{
	Window CreateWindow(const WindowId id, const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName,
		const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback, const CloseCallback& closeCallback);
	void SetCurrentContextWindow(Window& window);
}
#endif