#include "Utils/OpenGlUtils.hpp"

#ifdef OPENGL
#include <format>

namespace OpenGlUtils
{
	int GetTextureCubeFaceIndex(const Rendering::TextureCubeFace face)
	{
		return static_cast<int>(face);
	}

	GLenum GetStorage(const Rendering::AttachmentStorage storage)
	{
		if (storage == Rendering::AttachmentStorage::R8) return GL_R8;
		else if (storage == Rendering::AttachmentStorage::RGB8) return GL_RGB8;
		else if (storage == Rendering::AttachmentStorage::RGBA8) return GL_RGBA8;
		else if (storage == Rendering::AttachmentStorage::RGBA16F) return GL_RGBA16F;
		else if (storage == Rendering::AttachmentStorage::Depth24) return GL_DEPTH_COMPONENT24;
		else if (storage == Rendering::AttachmentStorage::Depth24_Stencil8) return GL_DEPTH24_STENCIL8;

		LogError(std::format("[OPENGL]: Attempted to convert channel format but texture channel format has no actions"));
		return 0;
	}
}
#endif