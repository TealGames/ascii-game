#pragma once
#include <functional>
#include "Utils/Data/Result.hpp"
#include <type_traits>
#include <utility>
#include "Utils/Data/Vec2Int.hpp"
#include "Utils/Data/Event.hpp"

namespace Core
{
	class Window;
	struct WindowPlatformCallbacks
	{
		std::function<BasicResult<bool>(Window&, int,int, const char*)> m_InitFunc;
		std::function<void(Window&)> m_UpdateFunc;
		std::function<void(Window&, int, int)> m_ResizeFunc;
		std::function<bool(Window&)> m_IsActiveFunc;
		std::function<void(Window&)> m_ShutdownFunc;

		bool HasAllValidCallbacks();
	};

	using UpdateCallbackType = std::function<void(Window&)>;

	/// <summary>
	/// This represents the viewport rect based in the window space
	/// where bottom left is (0,0) x increases RIGHT y increase UP
	/// </summary>
	struct WindowViewportRect
	{
		Vec2Int m_StartPos;
		Vec2Int m_Size;
	};

	class Window
	{
	private:
		WindowPlatformCallbacks m_platformCallbacks;
		UpdateCallbackType m_updateCallback;
		void* m_nativeState;

		Vec2Int m_size;
		Vec2Int m_aspectRatioConstraint;
		const char* m_windowName;

	public:
		static const Vec2Int NO_ASPECT_RATIO_CONSTRAINT;

		Event<void, Vec2Int> m_OnResize;

	private:
		BasicResult<bool> Init(const int width, const int height, const char* windowName);
	public:
		Window(const int width, const int height, const Vec2Int aspectRatioConstraint, const char* windowName, const bool hasNativeState,
			const WindowPlatformCallbacks& callbacks, const UpdateCallbackType& updateCallback);
		~Window();

		bool HasValidPlatformCallbacks();
		bool IsValid();

		void SetUpdateCallback(const UpdateCallbackType& callback);
		void Update();

		bool IsActive();
		void Shutdown();

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

		WindowViewportRect CalculateViewportRect(const int newWidth, const int newHeight) const;

		std::string ToString() const;
	};
}