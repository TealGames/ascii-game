#pragma once
#include "Core/Window/Window.hpp"
#include <vector>
#include "Utils/Data/Event.hpp"

namespace Core
{
	class WindowManager
	{
	private:
		size_t m_windowLimit;
		std::vector<Window> m_windows;
	public:
		Event<void, Window*> m_OnWindowCreated;
		Event<void, Window*> m_OnWindowUpdated;
		Event<void, Window*, const WindowInputEventInfo> m_OnInput;

	private:
		void SetCurrentContextWindow(Window& window);
		void RegisterInput(Window& window, const WindowInputEventInfo& info);
	public:
		WindowManager();

		Window* CreateNewWindow(const int width, const int height, const Vec2Int constrainedApsectRatio, const char* name, 
			const UpdateCallbackType updateCallback=nullptr);

		/// <summary>
		/// Updates all windows and returns true if at leaast one was updated
		/// </summary>
		/// <returns></returns>
		void UpdateAllWindows(bool* allWindowsInactiveFlag=nullptr);
		void CloseAllWindows();
	};
}