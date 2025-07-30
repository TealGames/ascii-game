#pragma once
#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType);
		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t arraySize);

		VertexLayout CreateVertexLayout();
	}
}