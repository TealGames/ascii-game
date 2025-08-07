#include "Window.hpp"
#include "Core/Analyzation/Debug.hpp"
#include <format>
#include "Utils/HelperFunctions.hpp"

static constexpr bool DEFAULT_VSYNC_ENABLED = true;

namespace Core
{
	bool WindowPlatformCallbacks::HasAllValidCallbacks()
	{
		return m_InitFunc && m_UpdateFunc && m_IsActiveFunc && m_ShutdownFunc;
	}

	WindowInputEventInfo::WindowInputEventInfo(const ScreenPosition newCursorPos)
		: m_EventType(WindowInputEventType::MouseMove), m_NewCursorPos(newCursorPos) {}
	WindowInputEventInfo::WindowInputEventInfo(const Input::KeyCode keyCode, const Input::KeyState state)
		: m_EventType(WindowInputEventType::ButtonPress), m_KeyUpdated(keyCode), m_KeyState(state) {}

	const Vec2Int Window::NO_ASPECT_RATIO_CONSTRAINT = Vec2Int(-1, -1);

	Window::Window(const WindowId id, const int width, const int height, const Vec2Int aspectRatioConstraint, const char* windowName, 
		const WindowPlatformCallbacks& callbacks, const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback,
		const CloseCallback& closeCallback)
		: m_id(id), m_platformCallbacks(callbacks), m_nativeState(nullptr), m_updateCallback(updateCallback), m_inputEventCallback(inputCallback),
		m_size(width, height), m_windowName(windowName), m_aspectRatioConstraint(aspectRatioConstraint), m_vsyncEnabled(false), m_closeCallback(closeCallback)
	{
		
	}
	Window::Window(Window&& other) noexcept
	{
		m_id = std::exchange(other.m_id, -1);
		m_platformCallbacks = std::exchange(other.m_platformCallbacks, {});
		m_updateCallback = std::move(other.m_updateCallback);
		m_inputEventCallback = std::move(other.m_inputEventCallback);

		m_size = std::exchange(other.m_size, {});
		m_aspectRatioConstraint = std::exchange(other.m_aspectRatioConstraint, {});
		m_windowName= std::exchange(other.m_windowName, "");
		m_vsyncEnabled = std::exchange(other.m_vsyncEnabled, false);
		m_OnResize = std::move(m_OnResize);
		
		m_nativeState = other.m_nativeState;
		other.m_nativeState = nullptr;
	}

	Window::~Window()
	{
		if (m_nativeState == nullptr)
			return;

		delete m_nativeState;
	}
	WindowId Window::GetId() const { return m_id; }

	void Window::RegisterInput(const WindowInputEventInfo& info)
	{
		if (m_inputEventCallback == nullptr)
			return;

		m_inputEventCallback(*this, info);
	}

	bool Window::HasValidPlatformCallbacks()
	{
		return m_platformCallbacks.HasAllValidCallbacks();
	}
	bool Window::IsValid()
	{
		return HasValidPlatformCallbacks() && m_nativeState != nullptr;
	}

	void Window::SetUpdateCallback(const UpdateCallbackType& callback)
	{
		m_updateCallback = callback;
	}
	void Window::Update()
	{
		m_platformCallbacks.m_UpdateFunc(*this);

		if (m_updateCallback)
			m_updateCallback(*this);
	}

	void Window::Init()
	{
		if (!m_platformCallbacks.m_InitFunc(*this, m_size.m_X, m_size.m_Y, m_windowName))
		{
			LogError(std::format("Failed to init window named:{} after executing init callback", m_windowName));
			return;
		}

		SetVSync(DEFAULT_VSYNC_ENABLED);
	}
	bool Window::IsActive()
	{
		return m_platformCallbacks.m_IsActiveFunc(*this);
	}
	void Window::Shutdown(const bool isLastWindow)
	{
		m_platformCallbacks.m_ShutdownFunc(*this, isLastWindow);
		if (m_closeCallback) m_closeCallback(*this);
	}
	void Window::SetNativeState(void* state)
	{
		m_nativeState = state;
	}
	void* Window::GetNativeStateMutable()
	{
		return m_nativeState;
	}
	void Window::SetSize(const int width, const int height)
	{
		m_size = Vec2Int(width, height);
		m_platformCallbacks.m_ResizeFunc(*this, width, height);
		m_OnResize.Invoke(Vec2Int(width, height));
	}

	Vec2Int Window::GetSize() const
	{
		return m_size;
	}
	const char* Window::GetWindowName() const
	{
		return m_windowName;
	}
	float Window::GetCurrentAspectRatio() const
	{
		return (float)m_size.m_X / m_size.m_Y;
	}

	bool Window::HasAspectRatioConstraint() const
	{
		return m_aspectRatioConstraint == NO_ASPECT_RATIO_CONSTRAINT;
	}
	void Window::SetAspectRatioConstraint(const Vec2Int constraint)
	{
		m_aspectRatioConstraint = constraint;
	}
	Vec2Int Window::GetAspectRatioConstraint() const
	{
		return m_aspectRatioConstraint;
	}

	void Window::SetVSync(bool enableVsync)
	{
		if (enableVsync == m_vsyncEnabled) return;

		m_vsyncEnabled = enableVsync;
		m_platformCallbacks.m_SetVsyncFunc(*this, enableVsync);
	}
	bool Window::IsVsyncEnabled() const
	{
		return m_vsyncEnabled;
	}

	WindowViewportRect Window::CalculateViewportRect(const int newWidth, const int newHeight) const
	{
		const float currRatio = (float)newWidth / newHeight;
		const float constrainedRatio = (float)m_aspectRatioConstraint.m_X / m_aspectRatioConstraint.m_Y;
		if (!HasAspectRatioConstraint() || Utils::ApproximateEqualsF(constrainedRatio, currRatio))
			return WindowViewportRect{ Vec2Int(0,0), Vec2Int(newWidth, newHeight) };

		Vec2Int offset = Vec2Int(0, 0);
		Vec2Int correctedSize = Vec2Int(newWidth, newHeight);

		//New width is bigger -> adds vertical bars on left/right
		if (currRatio > constrainedRatio)
		{
			correctedSize.m_X = newHeight * constrainedRatio;
			offset.m_X += (newWidth - correctedSize.m_X) / 2;
		}
		//New height is bigger -> add horizontal bars on top/bottom
		else
		{
			correctedSize.m_Y = newWidth * 1 / constrainedRatio;
			offset.m_Y += (newHeight - correctedSize.m_Y) / 2;
		}
		return WindowViewportRect{ offset, correctedSize};
	}
	Vec2Int Window::CalculateRenderSize() const
	{
		return CalculateViewportRect(m_size.m_X, m_size.m_Y).m_Size;
	}

	std::string Window::ToString() const
	{
		return std::format("[Window Size:{} Name:{}]", m_size.ToString(), m_windowName);
	}
}