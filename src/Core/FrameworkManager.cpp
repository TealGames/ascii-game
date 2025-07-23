#include "Core/FrameworkManager.hpp"
#include "StaticGlobals.hpp"

#ifdef GLFW
#include "GLFW/glfw3.h"
#endif

namespace Core
{
	BasicResult<bool> FrameworkInit()
	{
#ifdef GLFW
		if (!glfwInit())
			return "Failed to init GLFW";
#endif
		return true;
	}

	void FrameworkUpdate()
	{
#ifdef GLFW
		glfwPollEvents();
#endif
	}

	void FrameworkShutdown()
	{
#ifdef GLFW
		glfwTerminate();
#endif
	}
}