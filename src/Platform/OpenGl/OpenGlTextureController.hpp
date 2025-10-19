#pragma once
#include "Core/Rendering/TextureController.hpp"

#ifdef OPENGL
#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		TextureSlotController CreateTextureController();
		ImageSlotController CreateImageController();
	}
}
#endif