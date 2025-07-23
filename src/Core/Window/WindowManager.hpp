#pragma once
#include "Core/Window/Window.hpp"
#include <vector>
#include <functional>

namespace Core
{
	class WindowManager
	{
	private:
		size_t m_windowLimit;
		std::vector<Window> m_windows;
	public:

	private:
		void SetCurrentContextWindow(Window& window);
	public:
		WindowManager();

		Window* CreateNewWindow(const int width, const int height, const Vec2Int constrainedApsectRatio, const char* name, 
			const UpdateCallbackType& updateCallback=nullptr);

		/// <summary>
		/// Updates all windows and returns true if at leaast one was updated
		/// </summary>
		/// <returns></returns>
		void UpdateAllWindows(bool* allWindowsInactiveFlag=nullptr);
		void CloseAllWindows();
	};
}