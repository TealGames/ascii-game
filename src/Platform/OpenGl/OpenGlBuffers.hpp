#pragma once
#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		void CreateVertexBuffer(const VertexType* vertexArray, const size_t arraySize);
		void CreateIndexBuffer(const IndexType* indexArray, const size_t arraySize);
	}
}