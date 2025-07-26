#pragma once
#include "Core/Rendering/Shader/Shader.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		Shader CreateShader(const std::string& vertexShader, const std::string& fragmentSahder);
	}
}