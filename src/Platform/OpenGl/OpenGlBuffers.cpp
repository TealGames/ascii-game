#include "Platform/OpenGl/OpenGlBuffers.hpp"
#include "Utils/OpenGlUtils.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		static RenderObjectId AllocateVertexBuffer(const VertexType* vertexArray, const size_t arraySize)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateBuffers(1, id));
			GL_CALL(glNamedBufferData(id, arraySize * sizeof(VertexType), vertexArray, vertexArray!=nullptr? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
			return id;
		}
		static void WriteVertexBuffer(const RenderObjectId id, const size_t elementOffset, const VertexType* vertexArray, const size_t elementCount)
		{
			const size_t WRITE_BYTE_SIZE = elementCount * sizeof(VertexType);
			void* dataPtr = nullptr;
			GL_CALL(dataPtr = glMapNamedBufferRange(id, elementOffset * sizeof(VertexType), WRITE_BYTE_SIZE,
				//Note: WRITE BIT-> write operation, INVALIDATE_RAMGE -> deleting old memory, UNSYNCRHOZIED-> do not stall gpu while completing operation
				//TODO: for SYNCRHONIZATION BIT you must be sure no other read/write is occuring to this location (ENSURE THREAD SAFTETY)
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

			memcpy(dataPtr, static_cast<void*>(vertexArray), WRITE_BYTE_SIZE);
			GL_CALL(glUnmapNamedBuffer(id));
		}
		static void DeallocateVertexBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, id));
			return id;
		}

		void CreateVertexBuffer(const VertexType* vertexArray, const size_t arraySize)
		{
			return VertexBuffer(vertexArray, arraySize,
				VertexBufferPlatformCallbacks
				{
					AllocateVertexBuffer,
					WriteVertexBuffer
					DeallocateVertexBuffer
				});
		}

		static RenderObjectId AllocateIndexBuffer(const IndexType* indexArray, const size_t arraySize)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateBuffers(1, id));
			GL_CALL(glBufferData(id, arraySize * sizeof(IndexType), indexArray, indexArray != nullptr ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
			return id;
		}
		static void WriteIndexBuffer(const RenderObjectId id, const size_t elementOffset, const IndexType* vertexArray, const size_t elementCount)
		{
			const size_t WRITE_BYTE_SIZE = elementCount * sizeof(IndexType);
			void* dataPtr = nullptr;
			GL_CALL(dataPtr = glMapNamedBufferRange(id, elementOffset * sizeof(IndexType), WRITE_BYTE_SIZE,
				//Note: WRITE BIT-> write operation, INVALIDATE_RAMGE -> deleting old memory, UNSYNCRHOZIED-> do not stall gpu while completing operation
				//TODO: for SYNCRHONIZATION BIT you must be sure no other read/write is occuring to this location (ENSURE THREAD SAFTETY)
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

			memcpy(dataPtr, static_cast<void*>(vertexArray), WRITE_BYTE_SIZE);
			GL_CALL(glUnmapNamedBuffer(id));
		}
		static void DeallocateIndexBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, id));
			return id;
		}

		void CreateIndexBuffer(const IndexType* indexArray, const size_t arraySize)
		{
			return IndexBuffer(indexArray, arraySize, 
				IndexBufferPlatformCallbacks
				{
					AllocateIndexBuffer,
					WriteIndexBuffer,
					DeallocateIndexBuffer
				});
		}
	}

	

}