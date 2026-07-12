#pragma once
#include "Core/Window/Window.hpp"
#include <map>
#include "Utils/Data/Event.hpp"

namespace Engine::Core
{
	class WindowManager
	{
	private:
		size_t m_windowLimit;
		/// <summary>
		/// Stores the windows with the id
		/// </summary>
		std::map<WindowId, Window*> m_windows;
	public:
		Event<void, Window*> m_OnWindowCreated;
		Event<void, Window*> m_OnWindowUpdated;
		Event<void, Window*, const WindowInputEventInfo> m_OnInput;

	private:
		void SetCurrentContextWindow(Window& window);
		void RegisterInput(Window& window, const WindowInputEventInfo& info);
		WindowId GetNextAvailableWindowId() const;
	public:
		WindowManager();

		Window* CreateNewWindow(const int width, const int height, const Vec2Int constrainedApsectRatio, const char* name, 
			const UpdateCallbackType updateCallback=nullptr);

		void UpdateAllWindows();
		void CloseWindow(const WindowId id);
		void CloseAllWindows();

		size_t GetActiveWindowCount() const;
	};
}