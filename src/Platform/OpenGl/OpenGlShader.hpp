#pragma once

#ifdef OPENGL
#include "Core/Rendering/Shader/Shader.hpp"

namespace Engine::Rendering::OpenGl
{
	Shader CreateShader(const std::string& vertexShader, const std::string& fragmentSahder);
}
#endif