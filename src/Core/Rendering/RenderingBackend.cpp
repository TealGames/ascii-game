#include "Core/Rendering/RenderingBackend.hpp"
#include "StaticGlobals.hpp"
#include "EngineLog.hpp"
#include <ostream>

#ifdef OPENGL
#include "Utils/Platform/OpenGlUtils.hpp"
#include "Platform/OpenGl/OpenGlBuffers.hpp"
#include "Platform/OpenGl/OpenGlTextureController.hpp"
#endif

#ifdef GLFW
#include "GLFW/glfw3.h"
#endif

#ifdef RAYLIB
#include "raylib.h"
#include "Utils/RaylibUtils.hpp"
#endif 

static bool BackendLoaded = false;

namespace Rendering
{
	namespace Backend
	{
		void LoadBackend()
		{
#if defined(OPENGL) && defined(GLFW)
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				LogError("GLAD (OpenGL loader) init failed");
				return;
			}

			//Enable not culling back faces
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK); 
			//glFrontFace(GL_CCW);

			//The depth function for depth testing (gpu uses depth to determine what should be culled in framebuffer)
			//by comparing fragment z values (less means if it is less than existing fragment, it is culled)
			glDepthFunc(GL_LESS);
			SetDefaultDepthMode();

			//Enables alpha transparency
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			GL_CALL(glEnable(GL_DEBUG_OUTPUT));
			GL_CALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
			GL_CALL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
				0, nullptr, GL_TRUE));

			GL_CALL(glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)-> void
				{
					return;

					const char* sourceStr = "NULL";
					if (source == GL_DEBUG_SOURCE_API) sourceStr = "OpenGlAPI";
					else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM) sourceStr = "Window";
					else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER) sourceStr = "ShaderCompiler";
					else if (source == GL_DEBUG_SOURCE_THIRD_PARTY) sourceStr = "ThirdParty";
					else if (source == GL_DEBUG_SOURCE_APPLICATION) sourceStr = "MyDebugMessage";
					else if (source == GL_DEBUG_SOURCE_OTHER) sourceStr = "Other";

					const char* debugTypeStr = "NULL";
					if (type == GL_DEBUG_TYPE_ERROR) debugTypeStr = "Error";
					else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) debugTypeStr = "DeprecatedBehavior";
					else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) debugTypeStr = "UndefinedBehavior";
					else if (type == GL_DEBUG_TYPE_PORTABILITY) debugTypeStr = "NonPortableBehavior";
					else if (type == GL_DEBUG_TYPE_PERFORMANCE) debugTypeStr = "Performance";
					else if (type == GL_DEBUG_TYPE_MARKER) debugTypeStr = "CommandStreamAnnotation";
					else if (type == GL_DEBUG_TYPE_PUSH_GROUP) debugTypeStr = "PushGroupMessage";
					else if (type == GL_DEBUG_TYPE_POP_GROUP) debugTypeStr = "PopGroupMessage";
					else if (type == GL_DEBUG_TYPE_OTHER) debugTypeStr = "Other";

					LogType logType = LogType::Log;
					const char* severityStr = "NULL";
					if (severity == GL_DEBUG_SEVERITY_HIGH)
					{
						severityStr = "High";
						logType = LogType::Error;
					}
					else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
					{
						severityStr = "Medium";
						logType = LogType::Error;
					}
					else if (severity == GL_DEBUG_SEVERITY_LOW)
					{
						severityStr = "Low";
						logType = LogType::Warning;
					}
					else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) severityStr = "Notifcation";

					const std::string formattedMessage = std::format("[OPENGL] [Source:{} | Type:{} | Severity:{}]: {}",
						sourceStr, debugTypeStr, severityStr, message);

					if ((logType & LogType::Error) != 0) LogError(formattedMessage);
					else if ((logType & LogType::Warning) != 0) LogWarning(formattedMessage);
					else Log(formattedMessage);

				}, nullptr));
#else
			LogError("Attempted to load rendering backend but either no rendering library is active or it has no defined actions");
#endif

			BackendLoaded = true;
			Core::EngineLog(std::format("LOADED RENDER BACKEND:{}", GetBackendVersion()));
		}
		bool IsBackendLoaded()
		{
			return BackendLoaded;
		}
		const std::string GetBackendVersion()
		{
			if (!BackendLoaded)
				return "[Backend_Not_Loaded]";

#if defined(OPENGL)
			std::ostringstream oss;
			oss << glGetString(GL_VERSION);
			return "OpenGL "+ oss.str();
#endif
		}

		void SetViewport(const int x, const int y, const int width, const int height)
		{
#if defined(OPENGL)
			glViewport(x, y, width, height);
#endif
		}
		void SetViewport(const int width, const int height)
		{
			SetViewport(0, 0, width, height);
		}
		Vec2Int GetViewportSize()
		{
#if defined(OPENGL)
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			return Vec2Int(viewport[2], viewport[3]);
#endif
		}

		RenderBuffer CreateRenderBuffer(const TexelStorageType storage, const Vec2Int size)
		{
#if defined(OPENGL)
			return OpenGl::CreateRenderBuffer(storage, size);
#endif
		}
		FrameBuffer CreateFrameBuffer()
		{
#if defined(OPENGL)
			return OpenGl::CreateFrameBuffer();
#endif
		}

		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType)
		{
#if defined(OPENGL)
			return OpenGl::CreateVertexBuffer(vertexArray, elementSize, arraySize, advanceType);
#endif
		}
		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t elementCount)
		{
#if defined(OPENGL)
			return OpenGl::CreateIndexBuffer(indexArray, elementCount);
#endif
		}
		UniformBuffer CreateUniformBuffer(const char* blockName)
		{
#if defined(OPENGL)
			return OpenGl::CreateUniformBuffer(blockName);
#endif
		}
		ShaderStorageBuffer CreateShaderStorageBuffer(const char* blockName)
		{
#if defined(OPENGL)
			return OpenGl::CreateShaderStorageBuffer(blockName);
#endif
		}

		VertexLayout CreateVertexLayout()
		{
#if defined(OPENGL)
			return OpenGl::CreateVertexLayout();
#endif
		}

		TextureSlotController CreateTextureController()
		{
#if defined(OPENGL)
			return OpenGl::CreateTextureController();
#endif
		}
		ImageSlotController CreateImageController()
		{
#if defined(OPENGL)
			return OpenGl::CreateImageController();
#endif
		}

		RenderObjectId GetRenderObjectId(const RenderObjectQueryType type)
		{
			GLint currentFBO = INVALID_OBJ_ID;
#if defined(OPENGL)
			if (type == RenderObjectQueryType::BoundFrameBuffer) 
			{
				GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO));
			}
#else
#error "No rendering library is active"
#endif
			/*if (currentFBO == INVALID_OBJ_ID)
				LogError(std::format("Failed to get render object id of render object"));*/
			return currentFBO;
		}

		void BeginRenderingMarker()
		{
#if defined(RAYLIB)
			BeginDrawing();
#endif
		}

		void ClearBackground(std::uint8_t clearColorAttachments, const Color clearColor, const const float clearDepth)
		{
#if defined(OPENGL)
			const GLfloat* clearColorArr = reinterpret_cast<const GLfloat*>(clearColor.GetMemPointer());

			//GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			for (std::uint8_t i = 0; i < 8; i++)
			{
				std::uint8_t colorBit = clearColorAttachments & (1 << i);
				if (colorBit != 0) glClearBufferfv(GL_COLOR, i, clearColorArr);
			}

			GL_CALL(glClearDepth(1));
			ClearBufferBit(BufferBitType::Depth);

#elif defined(RAYLIB)
			ClearBackground(BLACK);
#else
			LogError("Attempted to clear canvas but either no rendering library is active or it has no defined actions");
#endif
		}
		void ClearBufferBit(const BufferBitType bitType)
		{
#if defined(OPENGL)
			GLbitfield bitField = 0;
			if (bitType == BufferBitType::Color) bitField |= GL_COLOR_BUFFER_BIT;
			if (bitType == BufferBitType::Depth) bitField |= GL_DEPTH_BUFFER_BIT;

			GL_CALL(glClear(bitField));
#else
			LogError("Attempted to clear buffer bit but either no rendering library is active or it has no defined actions");
#endif
		}

		void SetDepthWriting(const bool enable)
		{
#if defined(OPENGL)
			if (enable) GL_CALL(glDepthMask(GL_TRUE));
			else GL_CALL(glDepthMask(GL_FALSE));
#endif
		}
		void SetDepthTesting(const bool enable)
		{
#if defined(OPENGL)
			if (enable) GL_CALL(glEnable(GL_DEPTH_TEST));
			else GL_CALL(glDisable(GL_DEPTH_TEST));
#endif
		}

		void SetDepthMode(const DepthMode mode)
		{
#if defined(OPENGL)
			SetDepthWriting((mode & DepthMode::Write) != 0);
			SetDepthTesting((mode & DepthMode::Test) != 0);
#endif
		}

		void SetDefaultDepthMode()
		{
			SetDepthMode(DepthMode::Write | DepthMode::Test);
		}

		void InvokeImageMemorySync(const ImageOperationBarrierType barrier)
		{
#if defined(OPENGL)
			GLbitfield barrierBit = 0;

			if (barrier == ImageOperationBarrierType::ImageAccess)
				barrierBit = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
			else if (barrier == ImageOperationBarrierType::TextureFetch)
				barrierBit = GL_TEXTURE_FETCH_BARRIER_BIT;
			else if (barrier == ImageOperationBarrierType::FrameBuffer)
				barrierBit = GL_FRAMEBUFFER_BARRIER_BIT;
			else if (barrier == ImageOperationBarrierType::All)
				barrierBit = GL_ALL_BARRIER_BITS;
			else
			{
				LogError("Attempted to invoke image memory sync but barrier type has no actions defined");
				return;
			}

			GL_CALL(glMemoryBarrier(barrierBit));
#endif
		}
		void SetSrgbConversionStatus(const bool enable)
		{
#if defined(OPENGL)
			if (enable)
			{
				GL_CALL(glEnable(GL_FRAMEBUFFER_SRGB));
			}
			else
			{
				GL_CALL(glDisable(GL_FRAMEBUFFER_SRGB));
			}
#endif
		}

		void EndRenderingMarker()
		{
#if defined(OPENGL)
			//glClearColor(1, 0, 0, 1); 
			//glClear(GL_COLOR_BUFFER_BIT);
#elif defined(RAYLIB)
			EndDrawing();
#endif
		}

		void DrawUploadedIndexBufferInstanced(const size_t& baseVertexIndex, const size_t& indicesStartByteOffset, 
			const size_t& drawIndexCount, const size_t& baseInstanceIndex, const size_t& drawInstanceCount)
		{
#if defined(OPENGL)
			//LogError("DRAWING");
			//int program = -1;
			//glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			//LogError(std::format("Current program: {}", program));

			//glActiveTexture(GL_TEXTURE0);

			/*GLint boundTex = 0;
			static GLint prevBound = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
			if (prevBound == 0) prevBound = boundTex;*/
			//LogWarning(std::format("RENDERING: Texture bound at slot:{} has id:{}", 0, boundTex));
			//if (boundTex==0 || boundTex != prevBound) LogError(std::format("tex changed and/or 0 id bound:{} prev:{}", boundTex, prevBound));

			GL_CALL(glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, drawIndexCount, GL_UNSIGNED_INT, 
				(const void*)indicesStartByteOffset, drawInstanceCount, baseVertexIndex, baseInstanceIndex));
#endif
		}

		void DrawVertices(const size_t vertexCount)
		{
#if defined(OPENGL)
			GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vertexCount));
#endif
		}
	}
}