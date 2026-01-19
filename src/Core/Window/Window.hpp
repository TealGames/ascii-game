#pragma once
#include <functional>
#include <type_traits>
#include <utility>
#include "Utils/Math/Vec2Type.hpp"
#include "Utils/Data/Event.hpp"
#include "Core/Input/InputKey.hpp"
#include "Utils/Math/ScreenPosition.hpp"

namespace Core
{
	enum class WindowCursorMode : std::uint8_t
	{
		/// <summary>
		/// The default cursor behavior -> 
		/// cursor is tracked with absolute pos and
		/// no longer recevies window events after leaving window bounds
		/// </summary>
		Normal		= 0,
		/// <summary>
		/// Same as normal mode, but the cursor is not rendered
		/// </summary>
		Hidden		= 1,
		/// <summary>
		/// Cursor is not tracked with absolute pos but with 
		/// deltas instead meaning that mouse deltas can be tracked even
		/// after they leave window bounds -> no cursor rendered
		/// </summary>
		Disabled	= 2,
	};

	enum class WindowAttribute : std::uint8_t
	{
		Focused			=0,
		Minimized		=1,
		Maximized		=2,
		Visible			=3,
		/// <summary>
		/// The cursor is above the visible window
		/// area
		/// </summary>
		Hovered			=4,
		/// <summary>
		/// The window is set to always be "on top"
		/// or floating
		/// </summary>
		Floating		=5,
		Resizable		=6,
	};

	class Window;
	struct WindowPlatformCallbacks
	{
		bool(*m_InitFunc)(Window&, int width, int height, const char* windowName);
		void(*m_UpdateFunc)(Window&);
		void(*m_ResizeFunc)(Window&, int width, int height);
		void(*m_SetVsyncFunc)(Window&, bool vsyncEnabled);
		void(*m_SetCursorFunc)(Window&, const WindowCursorMode cursorMode);
		bool(*m_IsActiveFunc)(Window&);
		bool(*m_HasAttributeFunc)(Window&, const WindowAttribute attribute);
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
	using CloseCallback = std::function<void(Window&)>;

	using WindowId = std::uint8_t;

	class Window
	{
	private:
		WindowId m_id;
		WindowPlatformCallbacks m_platformCallbacks;
		UpdateCallbackType m_updateCallback;
		InputEventCallbackType m_inputEventCallback;
		CloseCallback m_closeCallback;
		void* m_nativeState;

		Vec2Int m_size;
		Vec2Int m_aspectRatioConstraint;
		const char* m_windowName;
		bool m_vsyncEnabled;
		WindowCursorMode m_cursorMode;
	public:
		static const Vec2Int NO_ASPECT_RATIO_CONSTRAINT;

		Event<void, Vec2Int> m_OnResize;

	private:
	public:
		Window(const WindowId id, const int width, const int height, const Vec2Int aspectRatioConstraint, const char* windowName, 
			const WindowPlatformCallbacks& callbacks, const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback, 
			const CloseCallback& m_closeCallback);
		Window(const Window&) = delete;
		Window(Window&&) noexcept;
		~Window();

		void Init();

		WindowId GetId() const;

		void RegisterInput(const WindowInputEventInfo& info);
		void HandleFocus(bool isFocused);

		bool HasValidPlatformCallbacks();
		bool IsValid();

		void SetUpdateCallback(const UpdateCallbackType& callback);
		void Update();

		bool HasAttribute(const WindowAttribute attrib);
		/// <summary>
		/// If true, the window is open and running, otherwise
		/// the window needs to be shutdown
		/// </summary>
		/// <returns></returns>
		bool IsActive();
		/// <summary>
		/// If true, the window is the one selected by the user for input
		/// </summary>
		/// <returns></returns>
		bool IsFocused();
		void Shutdown(const bool isLastWindow);

		void SetNativeState(void* state);
		void* GetNativeStateMutable();
		/*template<typename T, typename...Args>
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
		}*/

		void SetSize(const int width, const int height);
		/// <summary>
		/// Will cause the window to get updated with the current set window dimensions
		/// </summary>
		void ForceSizeUpdate();

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

		void SetCursorMode(const WindowCursorMode mode);
		WindowCursorMode GetCursorMode() const;

		WindowViewportRect CalculateViewportRect(const int newWidth, const int newHeight) const;
		Vec2Int CalculateRenderSize() const;

		std::string ToString() const;

		Window& operator=(const Window&) = delete;
	};
}