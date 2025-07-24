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

	const Vec2Int Window::NO_ASPECT_RATIO_CONSTRAINT = Vec2Int(-1, -1);

	Window::Window(const int width, const int height, const Vec2Int aspectRatioConstraint, const char* windowName, const bool hasNativeState,
		const WindowPlatformCallbacks& callbacks, const UpdateCallbackType& updateCallback)
		: m_platformCallbacks(callbacks), m_nativeState(nullptr), m_updateCallback(updateCallback), 
		m_size(width, height), m_windowName(windowName), m_aspectRatioConstraint(aspectRatioConstraint), m_vsyncEnabled(false)
	{
		bool success = Init(width, height, windowName);
		if (!success)
		{
			LogError(std::format("Failed to init window named:{} after executing init callback", windowName));
			return;
		}
		if (hasNativeState && m_nativeState == nullptr)
		{
			LogError("Attempted to initialize window, but native state is null when window constructor has native state TRUE"
				"after invoking framework specific window init callback");
			return;
		}

		SetVSync(DEFAULT_VSYNC_ENABLED);
	}

	Window::~Window()
	{
		delete m_nativeState;
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

	bool Window::Init(const int width, const int height, const char* windowName)
	{
		return m_platformCallbacks.m_InitFunc(*this, width, height, windowName);
	}
	bool Window::IsActive()
	{
		return m_platformCallbacks.m_IsActiveFunc(*this);
	}
	void Window::Shutdown()
	{
		m_platformCallbacks.m_ShutdownFunc(*this);
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

	std::string Window::ToString() const
	{
		return std::format("[Window Size:{} Name:{}]", m_size.ToString(), m_windowName);
	}
}