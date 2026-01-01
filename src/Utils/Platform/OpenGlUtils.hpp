#pragma once
#include "StaticGlobals.hpp"

#ifdef OPENGL
#include "glad/glad.h"
#include "Utils/Debug.hpp"
#include "Core/Rendering/Texture.hpp"

namespace OpenGlUtils
{
	inline void GLClearError()
	{
		volatile int dummy = 0;
		while (glGetError() != GL_NO_ERROR)
		{
			dummy++;
		}
	}
	inline const char* GetErrorString(const GLenum err) 
	{
		switch (err) 
		{
			case GL_NO_ERROR: return "GL_NO_ERROR";
			case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
			case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
			case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
			case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_CONTEXT_LOST: return "GL_CONTEXT_LOST";
			default: return "UnknownError";
		}
	}

	inline bool GLLogCall(const char* function, const char* file, int line)
	{
		while (GLenum error = glGetError())
		{
			LogError(std::format("[OpenGL Error]:{} ({}): {} File:{} Line:{}", GetErrorString(error), error, function, file, line));
			return false;
		}
		return true;
	}

	int GetTextureCubeFaceIndex(const Rendering::TextureCubeFace face);
	GLenum GetStorage(const Rendering::TexelStorageType storage);
	GLenum GetAccessPermission(const Rendering::AccessPermissions permissions);
}

#define ASSERT(x) if (!(x)) __debugbreak();

#if defined(ENGINE_DEBUG)

#define GL_CALL(x)                                \
    do {                                         \
        OpenGlUtils::GLClearError();              \
        x;                                       \
        ASSERT(OpenGlUtils::GLLogCall(            \
            #x, __FILE__, __LINE__));             \
    } while (0)

#else

#define GL_CALL(x)                                \
    do {                                         \
        x;                                       \
    } while (0)

#endif

#endif

