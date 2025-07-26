#pragma once
#include "Core/Window/Window.hpp"

namespace Core
{
	constexpr size_t RAYLIB_WINDOW_LIMIT = 1;

	Window CreateRaylibWindow(const int width, const int height, const Vec2Int constrainedAspectRatio, const char* windowName, const UpdateCallbackType& updateCallback);
}