#pragma once
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/TextureController.hpp"

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
		void ClearBackground();
		void ClearDepth();
		void SetDepthStatus(const bool enable);
		void ClearColor();

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
		void EndRenderingMarker();

		void DrawCircle(const WorldPosition3D& pos, const float radius, const Color color);
		void DrawRectangle(const WorldPosition3D& pos, const Vec2& size, const Color color);
		void DrawTexture(const WorldPosition3D& destinationPos, const Vec2& destinationSize, const Vec2& sourcePos, const Vec2& sourceSize,
			const Texture& tex, const float rotation, const Color color);
		void DrawText(const WorldPosition3D& pos, const Font& font, const char* text, const float size, const float spacing, const Color color);

		void DrawLine(const WorldPosition3D& startPos, const WorldPosition3D& endPos, const float thickness, const Color color);
		void DrawRectangleLine(const WorldPosition3D& pos, const float thickness, const Vec2& size, const Color color);

		void DrawUploadedIndexBuffer(const size_t& indicesStartByteOffset, const size_t& drawIndexCount);
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