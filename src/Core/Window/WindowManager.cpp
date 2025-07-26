#include "Core/Window/WindowManager.hpp"
#include "StaticGlobals.hpp"
#include "Core/Analyzation/Debug.hpp"
#include <format>

#if defined(GLFW)
#include "Platform/GLFW/GlfwWindow.hpp"
#endif 

#if defined(RAYLIB)
#include "Platform/Raylib/RaylibWindow.hpp"
#endif 

namespace Core
{
	WindowManager::WindowManager() : m_windows(), m_windowLimit(-1), m_OnWindowCreated(), m_OnWindowUpdated()
	{ 
#if defined(RAYLIB)
		m_windowLimit = RAYLIB_WINDOW_LIMIT;
#endif
	}

	Window* WindowManager::CreateNewWindow(const int width, const int height, 
		const Vec2Int constrainedApsectRatio, const char* name, const UpdateCallbackType& updateCallback)
	{
		if (m_windows.size() >= m_windowLimit)
		{
			LogError(std::format("Attemted to create new window named:{} but max windows have been reached", name));
			return nullptr;
		}

#if defined(GLFW)
		m_windows.emplace_back(std::move(CreateGlfwWindow(width, height, constrainedApsectRatio, name, updateCallback)));
#elif defined(RAYLIB)
		m_windows.emplace_back(std::move(CreateRaylibWindow(width, height, constrainedApsectRatio, name, updateCallback)))
#else
		LogError(std::format("Attempted to create a new window of size:{}x{} named:{} but the active framework has no actions, "
			"is not supported or no active frameworks were selected", width, height, name));
		return nullptr;
#endif
		Window* windowCreated = &(m_windows.back());;
		m_OnWindowCreated.Invoke(windowCreated);
		return windowCreated;
	}

	void WindowManager::SetCurrentContextWindow(Window& window)
	{
#if defined(GLFW)
		SetCurrentContextWindow(window);
#elif defined(RAYLIB)
		return;
#else
		LogError(std::format("Attempted to set the current context window as{} but the active framework has no actions, "
			"is not supported or no active frameworks were selected", window.ToString()));
#endif
	}

	void WindowManager::UpdateAllWindows(bool* allWindowsInactiveFlag)
	{
		if (m_windows.empty()) return;

		int inactiveCount = 0;
		for (auto& window : m_windows)
		{
			if (!window.IsActive())
			{
				inactiveCount++;
				continue;
			}

			SetCurrentContextWindow(window);
			m_OnWindowUpdated.Invoke(&window);
			window.Update();
		}
		if (allWindowsInactiveFlag != nullptr)
			*allWindowsInactiveFlag = inactiveCount == m_windows.size();

	}

	void WindowManager::CloseAllWindows()
	{
		if (m_windows.empty()) return;

		for (auto& window : m_windows)
		{
			window.Shutdown();
		}
	}
}