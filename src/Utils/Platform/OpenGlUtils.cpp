#include "Utils/Platform/OpenGlUtils.hpp"

#ifdef OPENGL
#include <format>

namespace OpenGlUtils
{
	using namespace Engine;
	int GetTextureCubeFaceIndex(const Rendering::TextureCubeFace face)
	{
		return static_cast<int>(face);
	}

	GLenum GetStorage(const Rendering::TexelStorageType storage)
	{
		if (storage == Rendering::TexelStorageType::R8) return GL_R8;
		else if (storage == Rendering::TexelStorageType::RGB8) return GL_RGB8;
		else if (storage == Rendering::TexelStorageType::RGB16F) return GL_RGB16F;
		else if (storage == Rendering::TexelStorageType::RGB32F) return GL_RGB32F;
		else if (storage == Rendering::TexelStorageType::RGBA8) return GL_RGBA8;
		else if (storage == Rendering::TexelStorageType::RGBA16F) return GL_RGBA16F;
		else if (storage == Rendering::TexelStorageType::RGBA32F) return GL_RGBA32F;
		else if (storage == Rendering::TexelStorageType::Depth24) return GL_DEPTH_COMPONENT24;
		else if (storage == Rendering::TexelStorageType::Depth24_Stencil8) return GL_DEPTH24_STENCIL8;

		LogError("[OPENGL]: Attempted to convert channel format but texture channel format has no actions");
		return 0;
	}

	GLenum GetAccessPermission(const Rendering::AccessPermissions permissions)
	{
		if (permissions == Rendering::AccessPermissions::ReadWrite)
			return GL_READ_WRITE;
		else if (permissions == Rendering::AccessPermissions::Read)
			return GL_READ_ONLY;
		else if (permissions == Rendering::AccessPermissions::Write)
			return GL_WRITE_ONLY;

		LogError("[OPENGL]: Attempted to get access permissions for a type with no defined actions");
		return 0;
	}
}
#endif