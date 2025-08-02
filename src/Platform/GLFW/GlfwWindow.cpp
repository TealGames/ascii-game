#include "GlfwWindow.hpp"

#ifdef GLFW
#include "GLFW/glfw3.h"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Core
{
	namespace Glfw
	{
		struct GlfwWindowState
		{
			GLFWwindow* m_Window;
			inline static bool m_GlfwInit = false;
		};

		Window CreateWindow(const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName,
			const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback)
		{
			return Window(width, height, aspectRatioCosntraint, windowName, true, WindowPlatformCallbacks
				{
					//Init
					[](Window& window, const int width, const int height, const char* windowName) -> bool
					{
						glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
						glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

						if (!GlfwWindowState::m_GlfwInit)
						{
							if (!glfwInit())
							{
								LogError("Failed to init GLFW");
								return false;
							}

							glfwSetErrorCallback([](int errorCode, const char* description)-> void
								{
									LogError(std::format("Encountered glfw error({}):{}", errorCode, description));
								});
							GlfwWindowState::m_GlfwInit = true;
						}

						GLFWwindow* glfwWindow = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
						if (glfwWindow == nullptr)
						{
							LogError("Failed to create GLFW window");
							return false;
						}
						GlfwWindowState& state = window.CreateNativeWindowState<GlfwWindowState>(glfwWindow);

						SetCurrentContextWindow(window);
						glfwSetWindowUserPointer(glfwWindow, &window);

						if (!Rendering::Backend::IsBackendLoaded())
							Rendering::Backend::LoadBackend();
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
						glfwSetCursorPosCallback(state.m_Window, [](GLFWwindow* glfwWindow, double xPos, double yPos) -> void
							{
								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								window->RegisterInput(WindowInputEventInfo(ScreenPosition(xPos, yPos)));
							});
						//NoteL modifier keys are keys like nums lock, caps lock, shift that can be pressed with another key ->
						//they recevie their own event, but when antoehr key (if any) has event, it sets modiferKeys args (so easier to parse multi-key presses)
						glfwSetKeyCallback(state.m_Window, [](GLFWwindow* glfwWindow, int key, int scanCode, int action, int modifierKeys) -> void
							{
								Input::KeyState keyState = Input::KeyState::Neutral;
								if (action == GLFW_PRESS) keyState = Input::KeyState::Pressed;
								else if (action == GLFW_REPEAT) keyState = Input::KeyState::Down;
								else if (action == GLFW_RELEASE) keyState = Input::KeyState::Released;

								if (keyState == Input::KeyState::Neutral)
									return;

								Input::KeyCode code = Input::KeyCode::A;
								//65 to 90 are the letter keys for glfw
								if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) key += (int)Input::KeyCode::A - GLFW_KEY_A;
								else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) key += (int)Input::KeyCode::F1 - GLFW_KEY_F1;

								code = static_cast<Input::KeyCode>(key);

								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								window->RegisterInput(WindowInputEventInfo(code, keyState));
							});

						return true;
					},
				//Update func
				[](Window& window) -> void
				{
					GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
					glfwSwapBuffers(state->m_Window);
					glfwPollEvents();
				},
				//Resize func
				[](Window& window, const int width, const int height) -> void
				{
					GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
					glfwSetWindowSize(state->m_Window, width, height);
				},
				//Set vsync func
				[](Window& window, const bool vsyncEnabled) -> void
				{
					GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
					if (vsyncEnabled) glfwSwapInterval(1);
					else glfwSwapInterval(0);
				},
				//IsActive
				[](Window& window)-> bool
				{
					GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
					return glfwWindowShouldClose(state->m_Window);
				},
				//Shutdown
				[](Window& window, const bool isLastWindow)-> void
				{
					GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
					if (state->m_Window != nullptr)
					{
						glfwDestroyWindow(state->m_Window);
						state->m_Window = nullptr;
					}
					if (isLastWindow) glfwTerminate();
				}
				}, updateCallback, inputCallback);
		}

		void SetCurrentContextWindow(Window& window)
		{
			GlfwWindowState* state = window.GetNativeStateMutable<GlfwWindowState>();
			const GLFWwindow* currentContextWindow = glfwGetCurrentContext();
			if (currentContextWindow == state->m_Window)
				return;

			glfwMakeContextCurrent(state->m_Window);
		}
	}
}
#endif