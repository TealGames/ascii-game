#include "Platform/Raylib/RaylibWindow.hpp"

#ifdef RAYLIB
#include "raylib.h"

namespace Core
{
	Window CreateRaylibWindow(const int width, const int height, const Vec2Int constrainedAspectRatio, const char* windowName, const UpdateCallbackType& updateCallback)
	{
		return Window(width, height, constrainedAspectRatio, windowName, false, WindowPlatformCallbacks
			{
				//Init
				[](Window& window, const int width, const int height, const char* windowName) -> bool
				{
					InitWindow(width, height, windowName);
					//TODO: maybe fps can be a core part of window init args?
					SetTargetFPS(TARGET_FPS);
					return true;
				},
			//Update
			[](Window& window) -> void
			{
				//TODO: implement
			},
			//Resize
			[](Window& window, const int width, const int height) -> void
			{
				//TODO: implement
			},
			//Set vsync func
			[](Window& window, const bool vsyncEnabled) -> void
			{
				//TODO: set vsync
			},
			//IsActive
			[](Window& window)-> bool
			{
				return WindowShouldClose();
			},
			//Shutdown
			[](Window& window, const bool isLastWindow)-> void
			{
				CloseWindow();
			}
			}, updateCallback);
	}
}
#endif