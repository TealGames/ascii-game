#include "Core/Window/WindowManager.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Debug.hpp"
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
	WindowId WindowManager::GetNextAvailableWindowId() const
	{
		//TODO: implement
		if (m_windows.empty())
			return 0;
		return (m_windows.end()--)->first + 1;
	}

	Window* WindowManager::CreateNewWindow(const int width, const int height, 
		const Vec2Int constrainedApsectRatio, const char* name, const UpdateCallbackType updateCallback)
	{
		if (m_windows.size() >= m_windowLimit)
		{
			LogError(std::format("Attemted to create new window named:{} but max windows have been reached", name));
			return nullptr;
		}

		Window* windowCreated = nullptr;
		const WindowId windowId = GetNextAvailableWindowId();

#if defined(GLFW)
		windowCreated = new Window(Glfw::CreateWindow(windowId, width, height, constrainedApsectRatio, name, updateCallback,
			[this](Window& window, const WindowInputEventInfo info) -> void {return RegisterInput(window, info); },
			[this](Window& window) -> void { CloseWindow(window.GetId()); }));
#elif defined(RAYLIB)
		m_windows.emplace_back(std::move(CreateRaylibWindow(width, height, constrainedApsectRatio, name, updateCallback)))
#else
		LogError(std::format("Attempted to create a new window of size:{}x{} named:{} but the active framework has no actions, "
			"is not supported or no active frameworks were selected", width, height, name));
		return nullptr;
#endif
		m_windows.emplace(windowId, windowCreated);

		//Note: init and constructor separated in case we need to pass on resources 
		//from one obj to another before it is actually created (to prevent null references in native state callbacks)
		windowCreated->Init();
		m_OnWindowCreated.Invoke(windowCreated);
		return windowCreated;
	}

	void WindowManager::SetCurrentContextWindow(Window& window)
	{
#if defined(GLFW)
		Glfw::SetCurrentContextWindow(window);
#elif defined(RAYLIB)
		return;
#else
		LogError(std::format("Attempted to set the current context window as{} but the active framework has no actions, "
			"is not supported or no active frameworks were selected", window.ToString()));
#endif
	}
	void WindowManager::RegisterInput(Window& window, const WindowInputEventInfo& info)
	{
		m_OnInput.Invoke(&window, info);
	}

	void WindowManager::UpdateAllWindows()
	{
		if (m_windows.empty()) return;

		int inactiveCount = 0;
		for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it)
		{
			if (!it->second->IsActive())
			{
				it->second->Shutdown(m_windows.size() == 1);
				continue;
			}

			SetCurrentContextWindow(*it->second);
			m_OnWindowUpdated.Invoke(it->second);

			//LogWarning("Updating window");
			it->second->Update();
		}
	}
	size_t WindowManager::GetActiveWindowCount() const
	{
		return m_windows.size();
	}
	void WindowManager::CloseWindow(const WindowId id)
	{
		delete m_windows[id];
		m_windows.erase(id);
	}
	void WindowManager::CloseAllWindows()
	{
		if (m_windows.empty()) return;

		for (auto& window : m_windows)
		{
			window.second->Shutdown(m_windows.size()==1);
		}
		m_windows.clear();
	}
}