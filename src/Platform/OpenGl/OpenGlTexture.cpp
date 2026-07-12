#include "Platform/OpenGl/OpenGlTexture.hpp"

#ifdef OPENGL
#include "Utils/Debug.hpp"
#include "Utils/Platform/OpenGlUtils.hpp"

namespace Engine::Rendering::OpenGl
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

	static GLenum GetInputFormat(const TexelChannelFormat channelFormat)
	{
		if (channelFormat == TexelChannelFormat::Single) return GL_RED;
		else if (channelFormat == TexelChannelFormat::RGB) return GL_RGB;
		else if (channelFormat == TexelChannelFormat::RGBA) return GL_RGBA;
		else if (channelFormat == TexelChannelFormat::Depth) return GL_DEPTH;
		else if (channelFormat == TexelChannelFormat::Depth_Stencil) return GL_DEPTH_STENCIL;

		LogError(std::format("[OPENGL]: Attempted to convert channel format but texture channel format has no actions"));
		return 0;
	}

	static GLenum GetChannelDataType(const TexelChannelDataType colorChannelType)
	{
		if (colorChannelType == TexelChannelDataType::Byte)
			return GL_UNSIGNED_BYTE;
		else if (colorChannelType == TexelChannelDataType::Float16)
			return GL_HALF_FLOAT;
		else if (colorChannelType == TexelChannelDataType::Float32)
			return GL_FLOAT;
		else if (colorChannelType == TexelChannelDataType::UInt32)
			return GL_UNSIGNED_INT;
		else if (colorChannelType == TexelChannelDataType::UInt24_8)
			return GL_UNSIGNED_INT_24_8;

		LogError(std::format("[OPENGL]: Attempted to convert internal storage to texel storage type"));
		return 0;
	}
	static void SetWrapBehavior(const RenderObjectId id, const AxesWrapBehavior wrap)
	{
		//Note: S-> x axis/U in texcoords, T-> y axis/V in tex, R-> z axis/w in tex
		GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_S, GetWrapBehavior(wrap[0])));
		GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_T, GetWrapBehavior(wrap[1])));
		GL_CALL(glTextureParameteri(id, GL_TEXTURE_WRAP_R, GetWrapBehavior(wrap[2])));
	}
	static void SetMinFilter(const RenderObjectId id, const MinFilter filter)
	{
		GL_CALL(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GetMinFilter(filter)));
	}
	static void SetMagFilter(const RenderObjectId id, const MagFilter filter)
	{
		GL_CALL(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GetMagFilter(filter)));
	}
	/// <summary>
	/// Will change the way the channel data is output when retrieving the color at texel
	/// coordinates. NOTE: this does NOT create additional data to the texture and only changes
	/// the way in which RGBA data is retrieved after sampling texture
	/// </summary>
	/// <param name="id"></param>
	/// <param name="channelOutputs"></param>
	static void SetSwizzleMask(const RenderObjectId id, const GLint(*channelOutputs)[4])
	{
		GL_CALL(glTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, *channelOutputs));
	}
	static void SetTextureSettings(const RenderObjectId id, const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
		//TODO: do something with it
		GL_CALL(glGenerateTextureMipmap(id));

		SetWrapBehavior(id, wrap);
		SetMinFilter(id, min);
		SetMagFilter(id, mag);
	}

	static RenderObjectId AllocateTexture(const TextureInfo& data)
	{
		RenderObjectId textureId;
		GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &textureId));
		GL_CALL(glTextureStorage2D(textureId, 1, OpenGlUtils::GetStorage(data.m_InternalStorage), data.m_TexelSize.m_X, data.m_TexelSize.m_Y));

		const std::uint8_t channelCount = GetChannelCount(data.m_InternalStorage);
		//If the texels are stored in a singular byte channel (NOTE: one channel always gets stored in RED)
		//it means the texture is most likely grayscale, so we apply red channel data to all channels
		if (channelCount == 1)
		{
			const GLint swizzleMask[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
			SetSwizzleMask(textureId, &swizzleMask);
		}
		else if (channelCount == 2)
		{
			const GLint swizzleMask[4] = { GL_RED, GL_GREEN, GL_ZERO, GL_ONE };
			SetSwizzleMask(textureId, &swizzleMask);
		}
		else if (channelCount == 3)
		{
			const GLint swizzleMask[4] = { GL_RED, GL_GREEN, GL_BLUE, GL_ONE };
			SetSwizzleMask(textureId, &swizzleMask);
		}

		SetTextureSettings(textureId, data.m_WrapBehavior, data.m_MinFilter, data.m_MagFilter);
		return textureId;
	}

	static void DeallocateTexture(const RenderObjectId id)
	{
		GL_CALL(glDeleteTextures(1, &id));
	}

	//NOTE: `texelOffset` is from the BOTTOM LEFT in [X,Y] (WIDTH, HEIGHT) and `texelSize` is in [WIDTH, HEIGHT]
	static void WriteBytes(const RenderObjectId id, const Vec2Int texelOffset, const Vec2Int texelSize,
		const TexelChannelFormat sourceFormat, const TexelChannelDataType sourceType, const std::byte* readPtr)
	{
		const GLenum channelFormat = GetInputFormat(sourceFormat);
		const GLenum channelDataType = GetChannelDataType(sourceType);
		//NOTE: OpenGL uses bottom left as (0,0) for offset
		GL_CALL(glTextureSubImage2D(id, 0, texelOffset.m_X, texelOffset.m_Y, texelSize.m_X, texelSize.m_Y, channelFormat, channelDataType, readPtr));
	}
	//`texelSize` is in [WIDTH, HEIGHT]
	static void CopyBytes(const RenderObjectId id, const Vec2Int texelSize, const Texture& otherTexture)
	{
		//Params: source id, source type, source mip map level, source offset X,Y,Z 
		// destination id, destination type, destination mip map level, destination offset, x,y,z destination size x,y,z
		// and last 3 are xyz start coords on texture to start copy from
		GL_CALL(glCopyImageSubData(otherTexture.GetId(), GL_TEXTURE_2D, 0, 0, 0, 0,
			id, GL_TEXTURE_2D, 0, 0, 0, 0, texelSize.m_X, texelSize.m_Y, 1));
	}

	//NOTE: `texelOffset` is from the BOTTOM LEFT in [X,Y] (WIDTH, HEIGHT) and `texelSize` is in [WIDTH, HEIGHT]
	static void ReadBytes(const RenderObjectId id, const Vec2Int texelOffset, const Vec2Int texelSize,
		const TexelChannelFormat sourceFormat, const TexelChannelDataType sourceType, std::byte* writePtr, const size_t bufferSize)
	{
		const GLenum channelFormat = GetInputFormat(sourceFormat);
		const GLenum channelDataType = GetChannelDataType(sourceType);
		/*(GLuint texture, GLint level, GLint xoffset, GLint yoffset,
			GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels);*/
			//NOTE: OpenGL uses bottom left as (0,0) for offset
		GL_CALL(glGetTextureSubImage(id, 0, texelOffset.m_X, texelOffset.m_Y, 0,
			texelSize.m_X, texelSize.m_Y, 1, channelFormat, channelDataType, bufferSize, writePtr));
	}

	Texture CreateTexture(const std::byte* data, const Vec2Int& size, const TextureBufferType type, const TexelStorageType storage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
		return Texture(data, size, type, storage, wrap, min, mag, TextureCallbacks
			{
				AllocateTexture,
				WriteBytes,
				SetWrapBehavior,
				SetMinFilter,
				SetMagFilter,
				CopyBytes,
				ReadBytes,
				DeallocateTexture,
			});
	}

	static RenderObjectId AllocateTextureCube(const TextureInfo& data)
	{
		RenderObjectId cubeId;
		GL_CALL(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubeId));
		//GL_CALL(glTextureStorage2D(GL_TEXTURE_2, 0, format, size.m_X, size.m_Y, 0, format, GL_UNSIGNED_BYTE, data));
		GL_CALL(glTextureStorage2D(cubeId, 1, OpenGlUtils::GetStorage(data.m_InternalStorage), data.m_TexelSize.m_X, data.m_TexelSize.m_Y));

		SetTextureSettings(cubeId, data.m_WrapBehavior, data.m_MinFilter, data.m_MagFilter);
		return cubeId;
	}
	static void SetDataCube(const TextureCubeFace face, const RenderObjectId id, const Vec2Int size, const TexelStorageType storage, const std::byte* data)
	{
		GLenum format = GetInputFormat(GetChannelFormatFromStorage(storage));
		GLenum texelStorage = GetChannelDataType(GetChannelDataTypeFromStorage(storage));
		GL_CALL(glTextureSubImage3D(id, 0, 0, 0, OpenGlUtils::GetTextureCubeFaceIndex(face), size.m_X, size.m_Y, 1, format, texelStorage, data));
	}

	TextureCube CreateTextureCube(const Vec2Int& size, const TexelStorageType storage,
		const AxesWrapBehavior wrap, const MinFilter min, const MagFilter mag)
	{
		return TextureCube(size, storage, wrap, min, mag, TextureCubeCallbacks
			{
				AllocateTextureCube,
				SetDataCube,
				DeallocateTexture,
			});
	}
}
#endif