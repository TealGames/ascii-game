#pragma once
#include <functional>
#include <type_traits>
#include <utility>
#include "Utils/Data/Vec2Type.hpp"
#include "Utils/Data/Event.hpp"
#include "Core/Input/InputKey.hpp"
#include "Utils/Data/ScreenPosition.hpp"

namespace Core
{
	class Window;
	struct WindowPlatformCallbacks
	{
		bool(*m_InitFunc)(Window&, int width, int height, const char* windowName);
		void(*m_UpdateFunc)(Window&);
		void(*m_ResizeFunc)(Window&, int width, int height);
		void(*m_SetVsyncFunc)(Window&, bool vsyncEnabled);
		bool(*m_IsActiveFunc)(Window&);
		void(*m_ShutdownFunc)(Window&, bool isLastWindow);

		bool HasAllValidCallbacks();
	};

	/// <summary>
	/// This represents the viewport rect based in the window space
	/// where bottom left is (0,0) x increases RIGHT y increase UP
	/// </summary>
	struct WindowViewportRect
	{
		Vec2Int m_StartPos;
		Vec2Int m_Size;
	};

	enum class WindowInputEventType : std::uint8_t
	{
		/// <summary>
		/// If the mouse position has updated
		/// </summary>
		MouseMove	= 0,
		/// <summary>
		/// If the mouse is being dragged
		/// </summary>
		MouseDrag	= 1,
		/// <summary>
		/// If any button is pressed (includes gamepad, keyboard, mouse buttons)
		/// </summary>
		ButtonPress	= 2,
	};

	struct WindowInputEventInfo
	{
		WindowInputEventType m_EventType = WindowInputEventType::MouseMove;
		ScreenPosition m_NewCursorPos = {};
		Input::KeyCode m_KeyUpdated = Input::KeyCode::Null;
		Input::KeyState m_KeyState = Input::KeyState::Neutral;

		WindowInputEventInfo(const ScreenPosition newCursorPos);
		WindowInputEventInfo(const Input::KeyCode keyCode, const Input::KeyState state);
	};

	using UpdateCallbackType = void(*)(Window&);
	using InputEventCallbackType = std::function<void(Window&, const WindowInputEventInfo& inputEventInfo)>;

	class Window
	{
	private:
		WindowPlatformCallbacks m_platformCallbacks;
		UpdateCallbackType m_updateCallback;
		InputEventCallbackType m_inputEventCallback;
		void* m_nativeState;

		Vec2Int m_size;
		Vec2Int m_aspectRatioConstraint;
		const char* m_windowName;
		bool m_vsyncEnabled;
	public:
		static const Vec2Int NO_ASPECT_RATIO_CONSTRAINT;

		Event<void, Vec2Int> m_OnResize;

	private:
		bool Init(const int width, const int height, const char* windowName);
	public:
		Window(const int width, const int height, const Vec2Int aspectRatioConstraint, const char* windowName, const bool hasNativeState,
			const WindowPlatformCallbacks& callbacks, const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback);
		Window(const Window&) = delete;
		Window(Window&&) noexcept;
		~Window();

		void RegisterInput(const WindowInputEventInfo& info);

		bool HasValidPlatformCallbacks();
		bool IsValid();

		void SetUpdateCallback(const UpdateCallbackType& callback);
		void Update();

		bool IsActive();
		void Shutdown(const bool isLastWindow);

		template<typename T, typename...Args>
		T& CreateNativeWindowState(Args&& ...args)
		{
			m_nativeState = static_cast<void*>(new T(std::forward<Args>(args)...));
			return *(GetNativeStateMutable<T>());
		}

		template<typename T>
		T* GetNativeStateMutable()
		{
			if (m_nativeState == nullptr) return nullptr;
			return static_cast<T*>(m_nativeState);
		}
		template<typename T>
		const T* GetNativeState() const
		{
			if (m_nativeState == nullptr) return nullptr;
			return static_cast<const T*>(m_nativeState);
		}

		void SetSize(const int width, const int height);

		Vec2Int GetSize() const;
		const char* GetWindowName() const;
		float GetCurrentAspectRatio() const;

		bool HasAspectRatioConstraint() const;
		void SetAspectRatioConstraint(const Vec2Int constraint);
		Vec2Int GetAspectRatioConstraint() const;

		/// <summary>
		/// Sets vertical syncrhonization. 
		/// If true -> will wait one refresh frame before swapping front/back buffers to render to screen
		/// If enabled can prevent screen tearing but may result in one frame input lag
		/// </summary>
		/// <param name="enableVsync"></param>
		void SetVSync(bool enableVsync);
		bool IsVsyncEnabled() const;

		WindowViewportRect CalculateViewportRect(const int newWidth, const int newHeight) const;
		Vec2Int CalculateRenderSize() const;

		std::string ToString() const;

		Window& operator=(const Window&) = delete;
	};
}