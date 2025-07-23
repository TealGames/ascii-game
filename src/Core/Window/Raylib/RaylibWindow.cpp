#include "Core/Window/Raylib/RaylibWindow.hpp"
#include "StaticGlobals.hpp"
#include "raylib.h"

namespace Core
{
	Window CreateRaylibWindow(const int width, const int height, const Vec2Int constrainedAspectRatio, const char* windowName, const UpdateCallbackType& updateCallback)
	{
		return Window(width, height, constrainedAspectRatio, windowName, false, WindowPlatformCallbacks
			{
				//Init
				[](Window& window, const int width, const int height, const char* windowName) -> BasicResult<bool>
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
			//IsActive
			[](Window& window)-> bool
			{
				return WindowShouldClose();
			},
			//Shutdown
			[](Window& window)-> void
			{
				CloseWindow();
			}
			}, updateCallback);
	}
}