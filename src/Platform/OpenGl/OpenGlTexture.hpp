#pragma once
#ifdef OPENGL
#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		Texture CreateTexture(const unsigned char* data, const Vec2Int& size, const ChannelFormat channelFormat, 
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag);
	}
}
#endif