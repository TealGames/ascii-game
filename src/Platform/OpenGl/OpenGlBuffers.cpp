#include "Platform/OpenGl/OpenGlBuffers.hpp"
#include <tuple>

#ifdef OPENGL
#include "Utils/OpenGlUtils.hpp"

#define PERMANENT_WRITE_PTR

namespace Rendering
{
	namespace OpenGl
	{
		static std::tuple<RenderObjectId, std::byte*> AllocateVertexBuffer(const void* vertexArray, const size_t totalByteSize, const bool allowPersistentReading)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateBuffers(1, &id));

#ifdef PERMANENT_WRITE_PTR
			//PERSISTENT -> ensures we get a valid cpu pointer for program lifetime, 
			//COHERENT -> writes are instantly visisble to the gpu (otherwise we need memory barriers/flushing)
			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			if (allowPersistentReading) flags |= GL_MAP_READ_BIT;
			GL_CALL(glNamedBufferStorage(id, totalByteSize, nullptr, flags));

			std::byte* dataPtr = nullptr;
			GL_CALL(dataPtr = static_cast<std::byte*>(glMapNamedBufferRange(id, 0, totalByteSize, flags)));
			return std::make_tuple(id, dataPtr);
#elif
			GL_CALL(glNamedBufferData(id, totalByteSize, vertexArray, vertexArray != nullptr ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
			return std::make_tuple(id, nullptr);
#endif
		}
		static void WriteVertexBuffer(const RenderObjectId id, const size_t byteOffset, 
			const void* vertexArray, const size_t totalByteSize)
		{
			void* dataPtr = nullptr;
			GL_CALL(dataPtr = glMapNamedBufferRange(id, byteOffset, totalByteSize,
				//Note: WRITE BIT-> write operation, INVALIDATE_RAMGE -> deleting old memory, UNSYNCRHOZIED-> do not stall gpu while completing operation
				//TODO: for SYNCRHONIZATION BIT you must be sure no other read/write is occuring to this location (ENSURE THREAD SAFTETY)
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

			if (dataPtr == nullptr)
			{
				LogError(std::format("[OPENGL]: Attempted to write to vertex buffer with id:{} "
					"but the dataptr retrieved to copy is null", id));
				return;
			}

			memcpy(dataPtr, vertexArray, totalByteSize);
			GL_CALL(glUnmapNamedBuffer(id));
			return;
		}
		static void DeallocateVertexBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, &id));
		}

		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType)
		{
			return VertexBuffer(vertexArray, elementSize, arraySize, advanceType,
				VertexBufferPlatformCallbacks
				{
					AllocateVertexBuffer,
					WriteVertexBuffer,
					DeallocateVertexBuffer
				});
		}


		static std::tuple<RenderObjectId,std::byte*> AllocateIndexBuffer(const IndexType* indexArray, 
			const size_t totalByteSize, const bool allowPersistentReading)
		{
			RenderObjectId id = INVALID_OBJ_ID;

			//NOTE: it may seem weird to use the direct state access functions for the vertex array, but not for index array creation
			//but OpenGL implicitly does extra stuff for index/element arrays -> any currently bound vertex array object (the object that controls
			//vertex layout and attributes) is automatically linked with an index buffer without requiring explicit linking
			//Since there is no other way to link an index buffer to a vertex array object we MUST do it this way AFTER creating the vertex array object
			//because the vertex array object ends up owning the index buffer comapred to a vertex buffer which is global state
			GL_CALL(glGenBuffers(1, &id));
			GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
			

#ifdef PERMANENT_WRITE_PTR
			//PERSISTENT -> ensures we get a valid cpu pointer for program lifetime, 
			//COHERENT -> writes are instantly visisble to the gpu (otherwise we need memory barriers/flushing)
			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			if (allowPersistentReading) flags |= GL_MAP_READ_BIT;
			GL_CALL(glNamedBufferStorage(id, totalByteSize, nullptr, flags));

			std::byte* dataPtr = nullptr;
			GL_CALL(dataPtr = static_cast<std::byte*>(glMapNamedBufferRange(id, 0, totalByteSize, flags)));
			return std::make_tuple(id, dataPtr);
#elif
			GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalByteSize, indexArray, indexArray != nullptr ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
			return std::make_tuple(id, nullptr);
#endif
		}
		static void WriteIndexBuffer(const RenderObjectId id, const size_t byteOffset, const IndexType* vertexArray, const size_t totalByteSize)
		{
			void* dataPtr = nullptr;
			GL_CALL(dataPtr = glMapNamedBufferRange(id, byteOffset, totalByteSize,
				//Note: WRITE BIT-> write operation, INVALIDATE_RAMGE -> deleting old memory, UNSYNCRHOZIED-> do not stall gpu while completing operation
				//TODO: for SYNCRHONIZATION BIT you must be sure no other read/write is occuring to this location (ENSURE THREAD SAFTETY)
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
			if (dataPtr == nullptr)
			{
				LogError(std::format("Attempted to write to index buffer with id:{} "
					"but the dataptr retrieved to copy is null", id));
				return;
			}
			memcpy(dataPtr, vertexArray, totalByteSize);
			GL_CALL(glUnmapNamedBuffer(id));
			return;
		}
		static void DeallocateIndexBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, &id));
		}

		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t arraySize)
		{
			return IndexBuffer(indexArray, arraySize, 
				IndexBufferPlatformCallbacks
				{
					AllocateIndexBuffer,
					WriteIndexBuffer,
					DeallocateIndexBuffer
				});
		}

		static RenderObjectId AllocateUniformBuffer(const size_t byteSize)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateBuffers(1, &id));
			GL_CALL(glNamedBufferData(id, byteSize, nullptr, GL_DYNAMIC_DRAW));
			return id;
		}
		static void BindUniformBuffer(const RenderObjectId id, const UniformBufferBindIndex bindIndex)
		{
			GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindIndex, id));
		}
		static void WriteUniformBuffer(const RenderObjectId id, const size_t byteOffset, const size_t writeByteSize, const void* data)
		{
			if (glIsBuffer(id) == GL_FALSE)
			{
				LogError(std::format("Attempted to write uniform buffer but id:{} is not a valid buffer", id));
				return;
			}
			GL_CALL(glNamedBufferSubData(id, byteOffset, writeByteSize, data));
		}
		static void DeallocateUniformBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, &id));
		}
		UniformBuffer CreateUniformBuffer()
		{
			return UniformBuffer(
				UniformBufferPlatformCallbacks
				{
					AllocateUniformBuffer,
					BindUniformBuffer,
					WriteUniformBuffer,
					DeallocateUniformBuffer
				});
		}

		static void InitVertexLayout(std::array<std::byte, IMPL_STATE_SIZE>& implState)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateVertexArrays(1, &id));
			GL_CALL(glBindVertexArray(id));
			implState = std::bit_cast<std::array<std::byte, IMPL_STATE_SIZE>>(id);
		}

		static void AddVertexLayoutAttribute(std::array<std::byte, IMPL_STATE_SIZE>& implState, const VertexAttribute& attribute)
		{
			const RenderObjectId id = std::bit_cast<RenderObjectId>(implState);
			GL_CALL(glEnableVertexArrayAttrib(id, attribute.m_ShaderLocation));

			GLuint componentType = GL_FLOAT;
			if (attribute.m_Type == VertexAttributeBaseType::Float)
				componentType = GL_FLOAT;
			else
			{
				LogError(std::format("Attempted to add vertex layout attribute "
					"but the component type has no corresponding opengl type"));
				return;
			}
			GL_CALL(glVertexArrayAttribFormat(id, attribute.m_ShaderLocation, attribute.m_ComponentCount, componentType, attribute.m_Normalize, attribute.m_ByteOffset));
			GL_CALL(glVertexArrayAttribBinding(id, attribute.m_ShaderLocation, attribute.m_BufferBindIndex));

			/*if (attribute.m_AdvanceType == VertexAttributeAdvance::Instance)
				GL_CALL(glVertexArrayBindingDivisor(id, attribute.m_ShaderLocation, 1));*/
		}

		static void BindBufferToVertexLayout(std::array<std::byte, IMPL_STATE_SIZE>& implState, const RenderObjectId bufferId, 
			const size_t elementSize, const VertexLayoutBindIndex bindIndex, const VertexAttributeAdvance advanceType)
		{
			const RenderObjectId vertexArrayObjId = std::bit_cast<RenderObjectId>(implState);
			if (vertexArrayObjId == INVALID_OBJ_ID || bufferId==INVALID_OBJ_ID)
			{
				LogError(std::format("OPENGL: Attempted to bind buffer:{} to vertex layout:{} "
					"but the buffer and/or vertex array object has invalid id", bufferId, vertexArrayObjId));
				return;
			}
			//LogError(std::format("buffer id:{} ({}) id:{}({}) element size:{} bindIndex:{}", bufferId, glIsBuffer(bufferId), vertexArrayObjId, glIsBuffer(vertexArrayObjId), elementSize, bindIndex));
			GL_CALL(glVertexArrayVertexBuffer(vertexArrayObjId, bindIndex, bufferId, 0, elementSize));

			if (advanceType == VertexAttributeAdvance::Instance)
			{
				GL_CALL(glVertexArrayBindingDivisor(vertexArrayObjId, bindIndex, 1));
			}
		}

		static void DeallocateVertexLayout(std::array<std::byte, IMPL_STATE_SIZE>& implState)
		{
			const RenderObjectId id = std::bit_cast<RenderObjectId>(implState);
			GL_CALL(glDeleteVertexArrays(1, &id));
		}

		VertexLayout CreateVertexLayout()
		{
			return VertexLayout(
				VertexLayoutCallbacks
				{
					InitVertexLayout,
					AddVertexLayoutAttribute,
					BindBufferToVertexLayout,
					DeallocateVertexLayout
				});
		}
	}
}
#endif