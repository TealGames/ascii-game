#include "Platform/OpenGl/OpenGlBuffers.hpp"
#include <tuple>

#ifdef OPENGL
#include "Utils/Platform/OpenGlUtils.hpp"

#define PERMANENT_WRITE_PTR

namespace Rendering
{
	namespace OpenGl
	{
		static RenderObjectId AllocateRenderBuffer(const TexelStorageType storage, const Vec2Int size)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateRenderbuffers(1, &id));
			GL_CALL(glNamedRenderbufferStorage(id, OpenGlUtils::GetStorage(storage), size.m_X, size.m_Y));

			return id;
		}
		static void DeallocateRenderBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteRenderbuffers(1, &id));
		}

		RenderBuffer CreateRenderBuffer(const TexelStorageType storage, const Vec2Int size)
		{
			return RenderBuffer(storage, size, RenderBufferPlatformCallbacks
				{
					AllocateRenderBuffer,
					DeallocateRenderBuffer
				});
		}

		static RenderObjectId AllocateFrameBufferFunc()
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateFramebuffers(1, &id));

			return id;
		}
		static void DeallocateFrameBufferFunc(const RenderObjectId id)
		{
			GL_CALL(glDeleteFramebuffers(1, &id));
		}
		static void BindActiveFrameBuffer(const RenderObjectId id, const size_t* colorAttachmentsArr, const size_t colorAttachmentsSize)
		{
			GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
			if (colorAttachmentsSize != 0)
			{
				GLenum* drawColorAttachments = (GLenum*)alloca(sizeof(GLenum) * colorAttachmentsSize);
				for (size_t i = 0; i < colorAttachmentsSize; i++)
				{
					drawColorAttachments[i] = GL_COLOR_ATTACHMENT0 + colorAttachmentsArr[i];
				}
				GL_CALL(glNamedFramebufferDrawBuffers(id, colorAttachmentsSize, drawColorAttachments));
			}
		}
		static void UnbindActiveFrameBuffer()
		{
			//NOTE: the default screen framebuffer (with attachments color, depth usually)
			//has id 0 -> rebind the default one that renders to screen
			GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
		static GLenum GetAttachmentType(const FrameBufferAttachmentType type)
		{
			if (MIN_COLOR_ATTACHMENT <= type && type <= MAX_COLOR_ATTACHMENT)
			{
				return GL_COLOR_ATTACHMENT0 + static_cast<FrameBufferAttachmentTypeIntegralType>(type)
					- static_cast<FrameBufferAttachmentTypeIntegralType>(MIN_COLOR_ATTACHMENT);
			}
			else if (type == FrameBufferAttachmentType::Depth) return GL_DEPTH_ATTACHMENT;
			else if (type == FrameBufferAttachmentType::Stencil) return GL_STENCIL_ATTACHMENT;
			else if (type == FrameBufferAttachmentType::DepthAndStencil) return GL_DEPTH_STENCIL_ATTACHMENT;
			else
			{
				LogError(std::format("[OPENGL]: Attempted to get attachment type but type does not have actions"));
				return 0;
			}
		}
		static void SetOutputTarget(const FrameBufferOutputTarget& target, const RenderObjectId id)
		{
			const GLenum attachmentType = GetAttachmentType(target.m_Type);
			if (target.m_TargetType == FrameBufferOutputType::Texture)
			{
				const FrameBufferTextureTarget& textureTarget = std::get<FrameBufferTextureTarget>(target.m_Targets);
				GL_CALL(glNamedFramebufferTexture(id, attachmentType, textureTarget.m_Texture->GetInfo().m_Id, 0));
			}
			else if (target.m_TargetType== FrameBufferOutputType::TextureCube)
			{
				const FrameBufferTextureCubeTarget& textureCubeTarget = std::get<FrameBufferTextureCubeTarget>(target.m_Targets);
				GL_CALL(glNamedFramebufferTextureLayer(id, attachmentType, textureCubeTarget.m_CubeTexture->GetData().m_Id, 0, 
					OpenGlUtils::GetTextureCubeFaceIndex(textureCubeTarget.m_Face)));
			}
			else if (target.m_TargetType == FrameBufferOutputType::RenderBuffer)
			{
				const FrameBufferRenderBufferTarget& renderBufferTarget = std::get<FrameBufferRenderBufferTarget>(target.m_Targets);
				GL_CALL(glNamedFramebufferRenderbuffer(id, attachmentType,
					GL_RENDERBUFFER, renderBufferTarget.m_RenderBuffer->GetId()));
			}
			else
			{
				LogError(std::format("[OPENGL]: Attempted to set output target but output target type has no actions"));
				return;
			}


			if (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				LogError(std::format("[OPENGL]: Attempted to set framebuffer output target but resulted in incomplete buffer"));
			}
		}
		static void RemoveOutputTarget(const FrameBufferOutputTarget& target, const RenderObjectId id)
		{
			const GLenum attachmentType = GetAttachmentType(target.m_Type);
			if (target.m_TargetType == FrameBufferOutputType::Texture)
			{
				GL_CALL(glNamedFramebufferTexture(id, attachmentType, 0, 0));
			}
			else if (target.m_TargetType == FrameBufferOutputType::TextureCube)
			{
				const FrameBufferTextureCubeTarget& textureCubeTarget = std::get<FrameBufferTextureCubeTarget>(target.m_Targets);
				GL_CALL(glNamedFramebufferTextureLayer(id, attachmentType, 0, 0,
					OpenGlUtils::GetTextureCubeFaceIndex(textureCubeTarget.m_Face)));
			}
			else if (target.m_TargetType == FrameBufferOutputType::RenderBuffer)
			{
				GL_CALL(glNamedFramebufferRenderbuffer(id, attachmentType, GL_RENDERBUFFER, 0));
			}
			else
			{
				LogError(std::format("[OPENGL]: Attempted to remove output target but output target type has no actions"));
				return;
			}
		}

		FrameBuffer CreateFrameBuffer()
		{
			return FrameBuffer(FrameBufferPlatformCallbacks
				{
					AllocateFrameBufferFunc,
					DeallocateFrameBufferFunc,
					BindActiveFrameBuffer,
					UnbindActiveFrameBuffer,
					SetOutputTarget,
					RemoveOutputTarget
				});
		}

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

			ENGINE_ASSERT(dataPtr != nullptr, "OPENGL: Attempted to write to vertex buffer with id:{} "
				"but the dataptr retrieved to copy is null", id);

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

			ENGINE_ASSERT(dataPtr != nullptr, "OEPNGL: Attempted to write to index buffer with id:{} "
				"but the dataptr retrieved to copy is null", id);
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
		static void BindUniformBuffer(const RenderObjectId id, const BufferBindIndex bindIndex)
		{
			GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindIndex, id));
		}
		static void WriteUniformBuffer(const RenderObjectId id, const size_t byteOffset, const size_t writeByteSize, const void* data)
		{
			ENGINE_ASSERT(data != nullptr, "OEPNGL: Attempted to WRITE uniform buffer but data pointer is NULL for buffer: {}", id);
			ENGINE_ASSERT(glIsBuffer(id) == GL_TRUE, "OEPNGL: Attempted to WRITE uniform buffer but id:{} is not a valid buffer", id);
			GL_CALL(glNamedBufferSubData(id, byteOffset, writeByteSize, data));
		}
		static void ReadUniformBuffer(const RenderObjectId id, const size_t byteOffset, const size_t readByteSize, void* writeData)
		{
			ENGINE_ASSERT(writeData != nullptr, "OPENGL: Attempted to READ uniform buffer but write data pointer is NULL for buffer: {}", id);
			ENGINE_ASSERT(glIsBuffer(id) == GL_TRUE, "OPENGL: Attempted to READ uniform buffer but id:{} is not a valid buffer", id);
			GL_CALL(glGetNamedBufferSubData(id, byteOffset, readByteSize, writeData));
		}

		static void DeallocateUniformBuffer(const RenderObjectId id)
		{
			GL_CALL(glDeleteBuffers(1, &id));
		}
		UniformBuffer CreateUniformBuffer(const char* blockName)
		{
			return UniformBuffer(blockName,
				ShaderBufferPlatformCallbacks
				{
					AllocateUniformBuffer,
					BindUniformBuffer,
					WriteUniformBuffer,
					ReadUniformBuffer,
					DeallocateUniformBuffer
				});
		}

		static RenderObjectId AllocateShaderStorageBuffer(const size_t byteSize)
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateBuffers(1, &id));
			GL_CALL(glNamedBufferData(id, byteSize, nullptr, GL_DYNAMIC_DRAW));
			return id;
		}
		static void BindShaderStorageBuffer(const RenderObjectId id, const BufferBindIndex bindIndex)
		{
			GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindIndex, id));
		}
		static void WriteShaderStorageBuffer(const RenderObjectId id, const size_t byteOffset, const size_t writeByteSize, const void* data)
		{
			ENGINE_ASSERT(data != nullptr, "OPENGL: Attempted to WRITE uniform buffer but data pointer is NULL for buffer: {}", id);
			ENGINE_ASSERT(glIsBuffer(id) == GL_TRUE, "OPENGL: Attempted to WRITE shader storage buffer but id:{} is not a valid buffer", id);
			GL_CALL(glNamedBufferSubData(id, byteOffset, writeByteSize, data));
		}
		static void ReadShaderStorageBuffer(const RenderObjectId id, const size_t byteOffset, const size_t readByteSize, void* writeData)
		{
			ENGINE_ASSERT(writeData != nullptr, "OPENGL: Attempted to READ uniform buffer but write data pointer is NULL for buffer: {}", id);
			ENGINE_ASSERT(glIsBuffer(id) == GL_TRUE, "OPENGL: Attempted to READ shader storage buffer but id:{} is not a valid buffer", id);
			GL_CALL(glGetNamedBufferSubData(id, byteOffset, readByteSize, writeData));
		}
		static void DeallocateShaderStorageBuffer(const RenderObjectId id)
		{
			LogWarning(std::format("Is buffer:{}", glIsBuffer(id) == GL_TRUE));
			GL_CALL(glDeleteBuffers(1, &id));
		}
		ShaderStorageBuffer CreateShaderStorageBuffer(const char* blockName)
		{
			return ShaderStorageBuffer(blockName,
				ShaderBufferPlatformCallbacks
				{
					AllocateShaderStorageBuffer,
					BindShaderStorageBuffer,
					WriteShaderStorageBuffer,
					ReadShaderStorageBuffer,
					DeallocateShaderStorageBuffer
				});
		}

		static RenderObjectId InitVertexLayout()
		{
			RenderObjectId id = INVALID_OBJ_ID;
			GL_CALL(glCreateVertexArrays(1, &id));
			
			return id;
		}

		static void BindActiveVertexLayout(const RenderObjectId id)
		{
			GL_CALL(glBindVertexArray(id));
		}
		static void UnbindActiveVertexLayout()
		{
			GL_CALL(glBindVertexArray(0));
		}

		static void AddVertexLayoutAttribute(const RenderObjectId id, const VertexAttribute& attribute)
		{
			GL_CALL(glEnableVertexArrayAttrib(id, attribute.m_ShaderLocation));

			GLint componentType = 0;
			if (attribute.m_Type == VertexAttributeBaseType::Float)
				componentType = GL_FLOAT;
			else if (attribute.m_Type == VertexAttributeBaseType::Integer)
				componentType = GL_INT;
			else if (attribute.m_Type == VertexAttributeBaseType::UnsignedInteger)
				componentType = GL_UNSIGNED_INT;
			else
			{
				LogError(std::format("Attempted to add vertex layout attribute "
					"but the component type has no corresponding opengl type"));
				return;
			}

			if (attribute.m_Type == VertexAttributeBaseType::Float)
			{
				GL_CALL(glVertexArrayAttribFormat(id, attribute.m_ShaderLocation,
					attribute.m_ComponentCount, componentType, attribute.m_Normalize, attribute.m_ByteOffset));
			}
			else
			{
				GL_CALL(glVertexArrayAttribIFormat(id, attribute.m_ShaderLocation,
					attribute.m_ComponentCount, componentType, attribute.m_ByteOffset));
			}
			
			GL_CALL(glVertexArrayAttribBinding(id, attribute.m_ShaderLocation, attribute.m_BufferBindIndex));

			/*if (attribute.m_AdvanceType == VertexAttributeAdvance::Instance)
				GL_CALL(glVertexArrayBindingDivisor(id, attribute.m_ShaderLocation, 1));*/
		}

		static void BindBufferToVertexLayout(const RenderObjectId id, const RenderObjectId bufferId, 
			const size_t elementSize, const VertexLayoutBindIndex bindIndex, const VertexAttributeAdvance advanceType)
		{
			//LogError(std::format("buffer id:{} ({}) id:{}({}) element size:{} bindIndex:{}", bufferId, 
			// glIsBuffer(bufferId), vertexArrayObjId, glIsBuffer(vertexArrayObjId), elementSize, bindIndex));
			GL_CALL(glVertexArrayVertexBuffer(id, bindIndex, bufferId, 0, elementSize));

			if (advanceType == VertexAttributeAdvance::Instance)
			{
				GL_CALL(glVertexArrayBindingDivisor(id, bindIndex, 1));
			}
		}

		static void DeallocateVertexLayout(const RenderObjectId id)
		{
			GL_CALL(glDeleteVertexArrays(1, &id));
		}

		VertexLayout CreateVertexLayout()
		{
			return VertexLayout(
				VertexLayoutCallbacks
				{
					InitVertexLayout,
					BindActiveVertexLayout,
					UnbindActiveVertexLayout,
					AddVertexLayoutAttribute,
					BindBufferToVertexLayout,
					DeallocateVertexLayout
				});
		}
	}
}
#endif