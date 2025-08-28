#pragma once
#ifdef OPENGL
#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		Texture CreateTexture(const std::byte* data, const Vec2Int& size, const InternalStorage storage,
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag);

		TextureCube CreateTextureCube(const Vec2Int& size, const InternalStorage storage,
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag);
	}
}
#endif