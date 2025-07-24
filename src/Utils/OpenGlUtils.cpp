#include "Utils/OpenGlUtils.hpp"
#include "Core/Analyzation/Debug.hpp"
#include <format>

namespace OpenGlUtils
{
	void GLClearError()
	{
		while(glGetError() != GL_NO_ERROR);
	}

	bool GLLogCall(const char* function, const char* file, int line)
	{
		while (GLenum error = glGetError())
		{
			LogError(std::format("[OpenGL Error]: ({}): {} File:{} Line:{}", error, function, file, line));
			return false;
		}
		return true;
	}
}
