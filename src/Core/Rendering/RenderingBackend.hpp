#pragma once
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/TextureController.hpp"
#include "Utils/HelperMacros.hpp"

namespace Rendering
{
	enum class RenderObjectQueryType : std::uint8_t
	{
		BoundFrameBuffer = 0
	};

	/// <summary>
	/// The type of barrier between an image operation and another graphics operation
	/// </summary>
	enum class ImageOperationBarrierType : std::uint8_t
	{
		/// <summary>
		/// Needed between compute passes that read/write from same image2d
		/// </summary>
		ImageAccess		= 0,
		/// <summary>
		/// Needed between an image operation and a texture sample
		/// </summary>
		TextureFetch	= 1,
		/// <summary>
		/// Needed between image operation and framebuffer use (draw call)
		/// </summary>
		FrameBuffer		= 2,
		/// <summary>
		/// Checks all barriers to ensure they are handled (safer, but slower)
		/// </summary>
		All				= 3
	};

	enum class BufferBitType : std::uint8_t
	{
		Color = 0,
		Depth = 1,
	};
	FLAG_ENUM_OPERATORS(BufferBitType)

	/// <summary>
	/// The type of depth settings enabled
	/// </summary>
	enum class DepthMode : std::uint8_t
	{
		/// <summary>
		/// Depth writing and testing will both be OFF
		/// </summary>
		None		= 0,
		/// <summary>
		/// If set, the depth positions will be written to the 
		/// depth buffer. By itself, this does nothing unless
		/// testing is also set
		/// </summary>
		Write		= 1,
		/// <summary>
		/// If set, will test fragments based on depth to determine
		/// if that fragment should be drawn or discarded. If this is set 
		/// and Write is OFF, then every fragment will be drawn, overriding previous
		/// fragments. If Write is ON, whether it gets drawn is determined by the
		/// set depth function when used to compare incoming fragment depth with the 
		/// current buffer depth for that pixel
		/// </summary>
		Test		= 1<<1,
		/// <summary>
		/// If set, both Write and Test will be ON
		/// </summary>
		All			= 0xff
	};
	FLAG_ENUM_OPERATORS(DepthMode)
	
	class Texture;
	class Font;
	namespace Backend
	{
		void LoadBackend();
		bool IsBackendLoaded();
		const std::string GetBackendVersion();

		void SetViewport(const int x, const int y, const int width, const int height);
		void SetViewport(const int width, const int height);
		Vec2Int GetViewportSize();
		
		RenderBuffer CreateRenderBuffer(const TexelStorageType storage, const Vec2Int size);
		FrameBuffer CreateFrameBuffer();
		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType);
		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t elementCount);
		UniformBuffer CreateUniformBuffer(const char* blockName);
		ShaderStorageBuffer CreateShaderStorageBuffer(const char* blockName);
		VertexLayout CreateVertexLayout();

		TextureSlotController CreateTextureController();
		ImageSlotController CreateImageController();

		RenderObjectId GetRenderObjectId(const RenderObjectQueryType type);

		void BeginRenderingMarker();
		/// <summary>
		/// Will set the clear color and clear depth and then clear both depth and color
		/// from the currently bound attachments of the currently bound framebuffer (or default framebuffer if none bound)
		/// NOTE: clearColorAttachments is a bitmask where bits correspond to color attachemnts (bit0 -> color0, bit1 -> color1, etc.)
		/// EXAMPLE: if we wanted to clear color0, color2 -> clearColorAttachments would be 00000101 (0b101)
		/// </summary>
		void ClearBackground(std::uint8_t clearColorAttachments, const HDRColor clearColor = HDRColor(0, 0, 0, 0), const const float clearDepth = 1);
		void ClearBufferBit(const BufferBitType bitType);

		void SetDepthMode(const DepthMode mode);
		void SetDefaultDepthMode();
		void SetDepthWriting(const bool enable);
		void SetDepthTesting(const bool enable);

		/// <summary>
		/// Since an Image data type can be invoked with parallel calls, those operations may not have 
		/// been flushed before another operation that uses that data is issued. Therefore, when transitioning
		/// between different image operations that use the same texture pixel data, this function must be called
		/// to ensure proper sync 
		/// </summary>
		void InvokeImageMemorySync(const ImageOperationBarrierType barrier);
		/// <summary>
		/// If true, will enable linear HDR -> sRGB conversion 
		/// (gamma curve applied to HDR colors so they look right since
		/// humans have non-linear brightness perception which is mimicked by monitor)
		/// </summary>
		/// <param name="enable"></param>
		void SetSrgbConversionStatus(const bool enable);
		/// <summary>
		/// Draws vertex buffer with index buffer and instance buffer info.
		/// NOTE: there is no offset for vertex buffer, so instead, just offset into index buffer and use basevertexindex
		/// of the size you want to go into vertex buffer
		/// </summary>
		/// <param name="baseVertexIndex">: Amount added to each index in index buffer</param>
		/// <param name="indicesStartByteOffset">: Byte offset into index buffer</param>
		/// <param name="drawIndexCount">: Number of indices to draw</param>
		/// <param name="baseInstanceIndex">: The starting index in the instance buffer</param>
		/// <param name="drawInstanceCount">: The number of instances to draw for instnace buffer</param>
		void DrawUploadedIndexBufferInstanced(const size_t& baseVertexIndex, const size_t& indicesStartByteOffset, const size_t& drawIndexCount, 
			const size_t& baseInstanceIndex, const size_t& drawInstanceCount);

		void DrawVertices(const size_t vertexCount);
	}
}