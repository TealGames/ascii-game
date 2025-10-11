#include "Platform/OpenGl/OpenGlTexture.hpp"

#ifdef OPENGL
#include "Utils/Debug.hpp"
#include "Utils/OpenGlUtils.hpp"
#include "Core/PositionConversions.hpp"

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

		static GLenum GetInputFormat(const ChannelFormat channelFormat)
		{
			if (channelFormat == ChannelFormat::Single) return GL_RED;
			else if (channelFormat == ChannelFormat::RGB) return GL_RGB;
			else if (channelFormat == ChannelFormat::RGBA) return GL_RGBA;
			else if (channelFormat == ChannelFormat::Depth) return GL_DEPTH;
			else if (channelFormat == ChannelFormat::Depth_Stencil) return GL_DEPTH_STENCIL;

			LogError(std::format("[OPENGL]: Attempted to convert channel format but texture channel format has no actions"));
			return 0;
		}

		static GLenum GetTexelStorageType(const AttachmentStorage storage)
		{
			if (storage == AttachmentStorage::R8 || storage == AttachmentStorage::RGB8 ||
				storage == AttachmentStorage::RGBA8)
				return GL_UNSIGNED_BYTE;
			//NOTE: technically, this needs to be GL_HALF_FLOAT
			//but since c++ does not have native 16 bit float we use 32
			else if (storage == AttachmentStorage::RGBA16F)
				return GL_FLOAT;
			else if (storage == AttachmentStorage::Depth24)
				return GL_UNSIGNED_INT;
			else if (storage == AttachmentStorage::Depth24_Stencil8)
				return GL_UNSIGNED_INT_24_8;

			LogError(std::format("[OPENGL]: Attempted to convert internal storage to texel storage type"));
			return 0;
		}

		static void SetTextureSettings(const RenderObjectId id, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
		{
			//TODO: do something with it
			GL_CALL(glGenerateTextureMipmap(id));

			//Note: S-> x axis/U in texcoords, T-> y axis/V in tex, R-> z axis/w in tex
			GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_S, GetWrapBehavior(wrap[0])));
			GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_T, GetWrapBehavior(wrap[1])));
			GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_R, GetWrapBehavior(wrap[2])));

			GL_CALL(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GetMinFilter(min)));
			GL_CALL(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GetMagFilter(mag)));
		}

		static RenderObjectId AllocateTexture(const TextureData& data)
		{
			RenderObjectId textureId;
			GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &textureId));
			//GL_CALL(glTextureStorage2D(GL_TEXTURE_2, 0, format, size.m_X, size.m_Y, 0, format, GL_UNSIGNED_BYTE, data));
			GL_CALL(glTextureStorage2D(textureId, 1, OpenGlUtils::GetStorage(data.m_internalStorage), data.m_size.m_X, data.m_size.m_Y));

			SetTextureSettings(textureId, data.m_wrapBehavior, data.m_minFilter, data.m_magFilter);
			return textureId;
		}

		static void DeallocateTexture(const RenderObjectId id)
		{
			GL_CALL(glDeleteTextures(1, &id));
		}

		static void SetData(const RenderObjectId id, const Vec2Int size, const AttachmentStorage storage, const std::byte* data) 
		{
			const GLenum format = GetInputFormat(GetChannelFormatFromStorage(storage));
			const GLenum texelStorage = GetTexelStorageType(storage);
			GL_CALL(glTextureSubImage2D(id, 0, 0, 0, size.m_X, size.m_Y, format, texelStorage, data));
		}

		static void GetData(const RenderObjectId id, const Vec2Int offset, const Vec2Int size,
			const AttachmentStorage storage, std::byte* writePtr, const size_t bufferSize)
		{
			const GLenum format = GetInputFormat(GetChannelFormatFromStorage(storage));
			const GLenum texelStorage = GetTexelStorageType(storage);
			const Vec2Int bottomLeftCenteredOffset = Conversions::TryToNewFixedAreaPos<int>(size, 
				CoordinateOriginType::TopLeft, offset + Vec2Int(0, size.m_Y), CoordinateOriginType::BottomLeft).value();

			//LogError(std::format("Get data offset:{} converted:{}", offset.ToString(), bottomLeftCenteredOffset.ToString()));
			GL_CALL(glGetTextureSubImage(id, 0, bottomLeftCenteredOffset.m_X, bottomLeftCenteredOffset.m_Y, 0, 
				size.m_X, size.m_Y, 1, format, texelStorage, bufferSize, writePtr));
		}

		static void SetBindStatus(const RenderObjectId id, const TextureSlotIndex index, const bool status)
		{
			if (index < 0 || index >= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
			{
				LogError(std::format("[OPENGL]: Attempted to set bind status for texture "
					"to index:{} status:{} but index is out of bounds", index, status));
				return;
			}

			if (status)
			{
				GL_CALL(glBindTextureUnit(index, id));
			}
			else
			{
				GL_CALL(glBindTextureUnit(index, 0));
			}
		}

		Texture CreateTexture(const std::byte* data, const Vec2Int& size, const AttachmentStorage storage,
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
		{
			return Texture(data, size, storage, wrap, min, mag, TextureCallbacks
				{
					AllocateTexture,
					SetData,
					GetData,
					SetBindStatus,
					DeallocateTexture,
				});
		}

		static RenderObjectId AllocateTextureCube(const TextureData& data)
		{
			RenderObjectId cubeId;
			GL_CALL(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubeId));
			//GL_CALL(glTextureStorage2D(GL_TEXTURE_2, 0, format, size.m_X, size.m_Y, 0, format, GL_UNSIGNED_BYTE, data));
			GL_CALL(glTextureStorage2D(cubeId, 1, OpenGlUtils::GetStorage(data.m_internalStorage), data.m_size.m_X, data.m_size.m_Y));

			SetTextureSettings(cubeId, data.m_wrapBehavior, data.m_minFilter, data.m_magFilter);
			return cubeId;
		}
		static void SetDataCube(const TextureCubeFace face, const RenderObjectId id, const Vec2Int size, const AttachmentStorage storage, const std::byte* data)
		{
			GLenum format = GetInputFormat(GetChannelFormatFromStorage(storage));
			GLenum texelStorage = GetTexelStorageType(storage);
			GL_CALL(glTextureSubImage3D(id, 0, 0, 0, OpenGlUtils::GetTextureCubeFaceIndex(face), size.m_X, size.m_Y, 1, format, texelStorage, data));
		}

		TextureCube CreateTextureCube(const Vec2Int& size, const AttachmentStorage storage, 
			const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
		{
			return TextureCube(size, storage, wrap, min, mag, TextureCubeCallbacks
				{
					AllocateTextureCube,
					SetDataCube,
					SetBindStatus,
					DeallocateTexture,
				});
		}
	}
}
#endif