#pragma once
#ifdef OPENGL
#include "Core/Rendering/Texture.hpp"

namespace Engine::Rendering::OpenGl
{
	Texture CreateTexture(const std::byte* data, const Vec2Int& texelSize, const TextureBufferType type, const TexelStorageType storage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag);

	TextureCube CreateTextureCube(const Vec2Int& texelSize, const TexelStorageType storage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag);
}
#endif 