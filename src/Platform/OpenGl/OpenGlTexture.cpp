#include "Platform/OpenGl/OpenGlTexture.hpp"

#ifdef OPENGL
#include "Core/Analyzation/Debug.hpp"
#include "Utils/OpenGlUtils.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		static GLint GetWrapBehavior(const WrapBehavior wrap)
		{
			if (wrap == WrapBehavior::Repeat)
				return GL_REPEAT;
			else if (wrap == WrapBehavior::MirroredRepeat)
				return GL_MIRRORED_REPEAT;
			else if (wrap == WrapBehavior::ClampEdge)
				return GL_CLAMP_TO_EDGE;
			else if (wrap == WrapBehavior::ClampBorder)
				return GL_CLAMP_TO_BORDER;

			LogError("Attempted to convert wrap behavior to OpenGL type but failed");
			return 0;
		}
		static GLint GetMinFilter(const MinFilter min)
		{
			if (min == MinFilter::Linear)
				return GL_LINEAR;
			else if (min == MinFilter::Nearest)
				return GL_NEAREST;
			else if (min == MinFilter::NearestMipmapNearest)
				return GL_NEAREST_MIPMAP_NEAREST;
			else if (min == MinFilter::NearestMipmapLinear)
				return GL_NEAREST_MIPMAP_LINEAR;
			else if (min == MinFilter::LinearMipmapNearest)
				return GL_LINEAR_MIPMAP_NEAREST;
			else if (min == MinFilter::LinaerMipmapLinear)
				return GL_LINEAR_MIPMAP_LINEAR;

			LogError("Attempted to convert min filter behavior to OpenGL type but failed");
			return 0;
		}
		static GLint GetMagFilter(const MagFilter mag)
		{
			if (mag == MagFilter::Linear)
				return GL_LINEAR;
			else if (mag == MagFilter::Nearest)
				return GL_NEAREST;

			LogError("Attempted to convert mag filter behavior to OpenGL type but failed");
			return 0;
		}

		static RenderObjectId AllocateTexture(const unsigned char* data, const Vec2Int& size, 
			const ChannelFormat channelFormat, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
		{
			RenderObjectId textureId;
			GL_CALL(glGenTextures(1, &textureId));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, textureId));

			GLenum format = GL_RGBA;
			if (channelFormat == ChannelFormat::Single) format = GL_RED;
			else if (channelFormat == ChannelFormat::RGB) format = GL_RGB;
			else if (channelFormat == ChannelFormat::RGBA) format = GL_RGBA;
			else
			{
				LogError(std::format("[OPENGL]: Attempted to allocate texture but texture channel format has no actions"));
				return INVALID_OBJ_ID;
			}
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, format, size.m_X, size.m_Y, 0, format, GL_UNSIGNED_BYTE, data));

			//TODO: do something with it
			GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
			
			//Note: S-> x axis/U in texcoords, T-> y axis/V in tex, R-> z axis/w in tex
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetWrapBehavior(wrap[0])));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetWrapBehavior(wrap[1])));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetWrapBehavior(wrap[2])));

			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetMinFilter(min)));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetMagFilter(mag)));

			return textureId;
		}
		static void DeallocateTexture(const RenderObjectId id)
		{
			GL_CALL(glDeleteTextures(1, &id));
		}

		static void BindToSlot(const RenderObjectId id, const TextureSlotIndex index)
		{
			GL_CALL(glActiveTexture(GL_TEXTURE0 + index));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, id));

			GLint boundTex = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
			//LogWarning(std::format("Texture bound at slot:{} has id:{}", index, boundTex));
		}
		static void UnbindFromSlot(const TextureSlotIndex index)
		{
			GL_CALL(glActiveTexture(GL_TEXTURE0 + index));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		}

		Texture CreateTexture(const unsigned char* data, const Vec2Int& size, const ChannelFormat format, 
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
		{
			return Texture(data, size, format, wrap, min, mag, TextureCallbacks
				{
					AllocateTexture,
					BindToSlot,
					UnbindFromSlot,
					DeallocateTexture,
				});
		}
	}
}
#endif