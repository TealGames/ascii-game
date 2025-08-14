#include "GlfwWindow.hpp"

#ifdef GLFW
#include "GLFW/glfw3.h"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Core
{
	namespace Glfw
	{
		inline static bool m_GlfwInit = false;

		Window CreateWindow (const WindowId id, const int width, const int height, const Vec2Int aspectRatioCosntraint, const char* windowName,
			const UpdateCallbackType updateCallback, const InputEventCallbackType& inputCallback, const CloseCallback& closeCallback)
		{
			return Window(id, width, height, aspectRatioCosntraint, windowName, WindowPlatformCallbacks
				{
					//Init
					[](Window& window, const int width, const int height, const char* windowName) -> bool
					{
						glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
						glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
						glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif
						//Enables depth in framebuffer for opengl ->
						//this allows opengl to not render pixels that are behind other pixels
						//in z space (note vertex shader still runs same amount -> fragment shader skips hidden pixels)
						//Note: 24 is a common option since most gpu have support + >16 <32 and is nice middleground
						glfwWindowHint(GLFW_DEPTH_BITS, 24);

						if (!m_GlfwInit)
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
							m_GlfwInit = true;
						}

						GLFWwindow* glfwWindow = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
						if (glfwWindow == nullptr)
						{
							LogError("Failed to create GLFW window");
							return false;
						}
						//GlfwWindowState& state = window.CreateNativeWindowState<GlfwWindowState>(glfwWindow);
						window.SetNativeState(glfwWindow);

						SetCurrentContextWindow(window);
						glfwSetWindowUserPointer(glfwWindow, &window);

						if (!Rendering::Backend::IsBackendLoaded())
							Rendering::Backend::LoadBackend();
						Rendering::Backend::SetViewport(0, 0, width, height);

						//Note: WE CANNOT USE CAPTURE GROUPS BECAUSE OF C-STyLE FUNC POINTERS IN GLFW
						glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow* glfwWindow , int w, int h) -> void
							{
								//glViewport(0, 0, w, h);
								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								WindowViewportRect viewportRect = window->CalculateViewportRect(w, h);
								Rendering::Backend::SetViewport(viewportRect.m_StartPos.m_X, viewportRect.m_StartPos.m_Y, viewportRect.m_Size.m_X, viewportRect.m_Size.m_Y);
							});

						glfwSetWindowFocusCallback(glfwWindow, [](GLFWwindow* glfwWindow, int focused)-> void
							{
								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								window->HandleFocus(focused == GLFW_TRUE);
							});

						glfwSetScrollCallback(glfwWindow, [](GLFWwindow*, double xOffset, double yOffset) -> void
							{
								//TODO: implement
							});
						glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow* glfwWindow, double xPos, double yPos) -> void
							{
								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								window->RegisterInput(WindowInputEventInfo(ScreenPosition(xPos, yPos)));
							});
						//NoteL modifier keys are keys like nums lock, caps lock, shift that can be pressed with another key ->
						//they recevie their own event, but when antoehr key (if any) has event, it sets modiferKeys args (so easier to parse multi-key presses)
						glfwSetKeyCallback(glfwWindow, [](GLFWwindow* glfwWindow, int key, int scanCode, int action, int modifierKeys) -> void
							{
								Input::KeyState keyState = Input::KeyState::Neutral;
								if (action == GLFW_PRESS) keyState = Input::KeyState::Pressed;
								else if (action == GLFW_REPEAT) keyState = Input::KeyState::Down;
								else if (action == GLFW_RELEASE) keyState = Input::KeyState::Released;
								else
								{
									LogError(std::format("[GLFW] Window attempted to register key input "
										"but action code:{} has no actions", action));
									return;
								}

								if (keyState == Input::KeyState::Neutral)
									return;

								Input::KeyCode code = static_cast<Input::KeyCode>(key);
								
								//65 to 90 are the letter keys for glfw
								/*if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) key += (int)Input::KeyCode::A - GLFW_KEY_A;
								else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) key += (int)Input::KeyCode::F1 - GLFW_KEY_F1;
								else if (key>= GLFW_KEY_RIGHT && key<= GLFW_KEY_UP) key+= (int)Input::KeyCode::ARR
								else
								{
									LogError(std::format("[GLFW] Window attempted to register key input "
										"but keyc code : {} has no actions", key));
									return;
								}*/

								/*code = static_cast<Input::KeyCode>(key);*/

								//LogWarning(std::format("Window has key:{} state:{}", ToString(keyState), ToString(code)));
								Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
								window->RegisterInput(WindowInputEventInfo(code, keyState));
							});

						return true;
					},
				//Update func
				[](Window& window) -> void
				{
					GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					glfwSwapBuffers(glfwWindow);
					glfwPollEvents();
				},
				//Resize func
				[](Window& window, const int width, const int height) -> void
				{
					GLFWwindow* glfwWIndow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					glfwSetWindowSize(glfwWIndow, width, height);
				},
				//Set vsync func
				[](Window& window, const bool vsyncEnabled) -> void
				{
					if (vsyncEnabled) glfwSwapInterval(1);
					else glfwSwapInterval(0);
				},
				//Set cursor mdoe func
				[](Window& window, const WindowCursorMode mode) -> void
				{
					int cursorMode = 0;
					if (mode == WindowCursorMode::Normal) cursorMode = GLFW_CURSOR_NORMAL;
					else if (mode == WindowCursorMode::Hidden) cursorMode = GLFW_CURSOR_HIDDEN;
					else if (mode == WindowCursorMode::Disabled) cursorMode = GLFW_CURSOR_DISABLED;

					GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					glfwSetInputMode(glfwWindow, GLFW_CURSOR, cursorMode);
				},
				//IsActive
				[](Window& window)-> bool
				{
					GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					return !glfwWindowShouldClose(glfwWindow);
				},
				//Has attribute
				[](Window& window, const WindowAttribute attribute)-> bool
				{
					GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					int attributeValue = 0;
					if (attribute == WindowAttribute::Focused) attributeValue = GLFW_FOCUSED;
					else if (attribute==WindowAttribute::Minimized) attributeValue = GLFW_ICONIFIED;
					else if (attribute==WindowAttribute::Maximized) attributeValue = GLFW_MAXIMIZED;
					else if (attribute==WindowAttribute::Visible) attributeValue = GLFW_VISIBLE;
					else if (attribute==WindowAttribute::Hovered) attributeValue = GLFW_HOVERED;
					else if (attribute==WindowAttribute::Floating) attributeValue = GLFW_FLOATING;
					else if (attribute==WindowAttribute::Resizable) attributeValue = GLFW_RESIZABLE;
					else
					{
						LogError(std::format("Attempted to check window attribute but it has no actions"));
						return false;
					}

					return glfwGetWindowAttrib(glfwWindow, attributeValue);
				},
				
				//Shutdown
				[](Window& window, const bool isLastWindow)-> void
				{
					GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
					if (glfwWindow != nullptr)
					{
						glfwDestroyWindow(glfwWindow);
						window.SetNativeState(nullptr);
					}
					if (isLastWindow) glfwTerminate();
				}
				}, updateCallback, inputCallback, closeCallback);
		}

		void SetCurrentContextWindow(Window& window)
		{
			GLFWwindow* targetGlfwWindow = static_cast<GLFWwindow*>(window.GetNativeStateMutable());
			if (glfwGetCurrentContext() == targetGlfwWindow)
				return;

			glfwMakeContextCurrent(targetGlfwWindow);
		}
	}
}
#endif