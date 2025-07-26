#pragma once
#include "glad/glad.h"
#include "Core/Analyzation/Debug.hpp"

namespace OpenGlUtils
{
	inline void GLClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}

	inline bool GLLogCall(const char* function, const char* file, int line)
	{
		while (GLenum error = glGetError())
		{
			LogError(std::format("[OpenGL Error]: ({}): {} File:{} Line:{}", error, function, file, line));
			return false;
		}
		return true;
	}
}

#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_CALL(x) do {\
	OpenGlUtils::GLClearError();\
	x;\
	ASSERT(OpenGlUtils::GLLogCall(#x, __FILE__, __LINE__))\
}while(0)

