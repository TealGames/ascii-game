#pragma once
#include "Core/Rendering/GpuFence.hpp"
#ifdef OPENGL

namespace Engine::Rendering::OpenGl
{
	GpuFence CreateGpuFence(const bool insertFence);
}

#endif