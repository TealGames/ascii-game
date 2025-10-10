#include "Core/Rendering/RenderingBackend.hpp"
#include "StaticGlobals.hpp"
#include "EngineLog.hpp"
#include <ostream>

#ifdef OPENGL
#include "Utils/OpenGlUtils.hpp"
#include "Platform/OpenGl/OpenGlBuffers.hpp"
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
			SetDepthStatus(true);

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

		RenderBuffer CreateRenderBuffer(const AttachmentStorage storage, const Vec2Int size)
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

		VertexLayout CreateVertexLayout()
		{
#if defined(OPENGL)
			return OpenGl::CreateVertexLayout();
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

		void ClearBackground()
		{
#if defined(OPENGL)
			GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

#elif defined(RAYLIB)
			ClearBackground(BLACK);
#else
			LogError("Attempted to clear canvas but either no rendering library is active or it has no defined actions");
#endif
		}
		void ClearDepth()
		{
#if defined(OPENGL)
			GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
#else
			LogError("Attempted to clear canvas but either no rendering library is active or it has no defined actions");
#endif
		}
		void SetDepthStatus(const bool enable)
		{
#if defined(OPENGL)
			if (enable)
			{
				//Enable depth testing -> if you draw triangles on top of one another, will resolve the one on bottom
				//based on position and not draw order
				GL_CALL(glEnable(GL_DEPTH_TEST));
			}
			else
			{
				GL_CALL(glDisable(GL_DEPTH_TEST));
			}
#endif
		}

		void ClearColor()
		{
#if defined(OPENGL)
			GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
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

		void DrawCircle(const WorldPosition3D& pos, const float radius, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawCircle(pos.m_X, pos.m_Y, radius, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawRectangle(const WorldPosition3D& pos, const Vec2& size, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawRectangle(pos.m_X, pos.m_Y, size.m_X, size.m_Y, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawTexture(const WorldPosition3D& destinationPos, const Vec2& destinationSize, const Vec2& sourcePos, const Vec2& sourceSize,
			const Texture& tex, const float rotation, const Utils::Color color)
		{
#if defined(RAYLIB)
			//TODO: there needs to be a way to get raylib texture from Texture
			DrawTexturePro(Texture2D(), Rectangle{sourcePos.m_X, sourcePos.m_Y, sourceSize.m_X, sourceSize.m_Y},
				Rectangle{destinationPos.m_X, destinationPos.m_Y, destinationSize.m_X, destinationSize.m_Y}, { 0, 0 }, rotation, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawText(const WorldPosition3D& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
		{
#if defined(RAYLIB)
			//Note: the text seems to flicker less when we put text on integer boundaries 
			//TODO: convert font to rayib font
			DrawTextEx(::Font(), text, RaylibUtils::ToRaylibVector(Vec2Int(pos.m_X, pos.m_Y)), size, spacing, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawLine(const WorldPosition3D& startPos, const WorldPosition3D& endPos, const float thickness, const Utils::Color color)
		{
#if defined(RAYLIB)
			//DrawLineEx(RaylibUtils::ToRaylibVector(startPos), RaylibUtils::ToRaylibVector(endPos), thickness, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawRectangleLine(const WorldPosition3D& pos, const float thickness, const Vec2& size, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawRectangleLinesEx(Rectangle{pos.m_X, pos.m_Y, size.m_X, size.m_Y }, thickness, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawUploadedIndexBuffer(const size_t& indicesStartByteOffset, const size_t& drawIndexCount)
		{
#if defined(OPENGL)
			GL_CALL(glDrawElements(GL_TRIANGLES, drawIndexCount, GL_UNSIGNED_INT, (const void*)indicesStartByteOffset));
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