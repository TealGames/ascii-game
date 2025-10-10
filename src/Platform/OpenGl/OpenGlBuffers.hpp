#pragma once
#ifdef OPENGL
#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		RenderBuffer CreateRenderBuffer(const AttachmentStorage storage, const Vec2Int size);
		FrameBuffer CreateFrameBuffer();
		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType);
		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t arraySize);
		UniformBuffer CreateUniformBuffer(const char* blockName);

		VertexLayout CreateVertexLayout();
	}
}
#endif