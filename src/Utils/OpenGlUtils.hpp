#pragma once
#include "glad/glad.h"

namespace OpenGlUtils
{
#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_CALL(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

	void GLClearError();
	bool GLLogCall(const char* function, const char* file, int line);
}

