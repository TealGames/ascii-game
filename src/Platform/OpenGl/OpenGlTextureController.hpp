#pragma once
#include "Core/Rendering/TextureController.hpp"

#ifdef OPENGL
#include "Core/Rendering/Buffers.hpp"

namespace Engine::Rendering::OpenGl
{
	TextureSlotController CreateTextureController();
	ImageSlotController CreateImageController();
}
#endif