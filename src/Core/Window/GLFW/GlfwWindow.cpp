#include "GlfwWindow.hpp"
#include "GLFW/glfw3.h"
#include "Core/Rendering/RenderingBackend.hpp"

namespace Core
{
	struct GlfwWindowState
	{
		GLFWwindow* m_Window = nullptr;
	};

	Window CreateGlfwWindow(const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName, const UpdateCallbackType& updateCallback)
	{
		return Window(width, height, aspectRatioCosntraint, windowName, true, WindowPlatformCallbacks
			{
				//Init
				[](Window& window, const int width, const int height, const char* windowName) -> BasicResult<bool>
				{
					glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
					glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
					glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

					GLFWwindow* glfwWindow = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
					if (glfwWindow == nullptr)
					{
						return "Failed to create GLFW window";
					}
					GlfwWindowState& state = window.CreateNativeWindowState<GlfwWindowState>(glfwWindow);
					SetCurrentContextWindow(window);
					glfwSetWindowUserPointer(glfwWindow, &window);

					Rendering::Backend::SetViewport(0, 0, width, height);

					//Note: WE CANNOT USE CAPTURE GROUPS BECAUSE OF C-STyLE FUNC POINTERS IN GLFW
					glfwSetWindowSizeCallback(state.m_Window, [](GLFWwindow* glfwWindow , int w, int h) -> void
						{
							//glViewport(0, 0, w, h);
							Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
							WindowViewportRect viewportRect = window->CalculateViewportRect(w, h);
							Rendering::Backend::SetViewport(viewportRect.m_StartPos.m_X, viewportRect.m_StartPos.m_Y, viewportRect.m_Size.m_X, viewportRect.m_Size.m_Y);
						});

					glfwSetScrollCallback(state.m_Window, [](GLFWwindow*, double xOffset, double yOffset) -> void
						{
							//TODO: implement
						});
					glfwSetCursorPosCallback(state.m_Window, [](GLFWwindow*, double xPos, double yPos) -> void
						{
							//TODO: implement
						});

					return true;
				},
			//Update func
			[](Window& window) -> void
			{
				GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
				glfwSwapBuffers(state->m_Window);
			},
			//Resize func
			[](Window& window, const int width, const int height) -> void
			{
				GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
				glfwSetWindowSize(state->m_Window, width, height);
			},
			//IsActive
			[](Window& window)-> bool
			{
				GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
				return glfwWindowShouldClose(state->m_Window);
			},
			//Shutdown
			[](Window& window)-> void
			{
				GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
				if (state->m_Window != nullptr)
				{
					glfwDestroyWindow(state->m_Window);
					state->m_Window = nullptr;
				}
			}
			}, updateCallback);
	}

	void SetCurrentContextWindow(Window& window)
	{
		GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
		glfwMakeContextCurrent(state->m_Window);
	}
}