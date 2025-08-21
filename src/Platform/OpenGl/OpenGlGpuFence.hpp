#pragma once
#include "Core/Rendering/GpuFence.hpp"
#ifdef OPENGL

namespace Rendering
{
	namespace OpenGl
	{
		GpuFence CreateGpuFence(const bool insertFence);
	}
}

#endif