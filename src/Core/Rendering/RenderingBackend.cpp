#include "Core/Rendering/RenderingBackend.hpp"
#include "StaticGlobals.hpp"
#include "Core/Analyzation/Debug.hpp"

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
#else
			LogError("Attempted to load rendering backend but either no rendering library is active or it has no defined actions");
#endif
			BackendLoaded = true;
		}
		bool IsBackendLoaded()
		{
			return BackendLoaded;
		}

		void SetViewport(const int x, const int y, const int width, const int height)
		{
#if defined(OPENGL)
			glViewport(x, y, width, height);
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

		VertexLayout CreateVertexLayout()
		{
#if defined(OPENGL)
			return OpenGl::CreateVertexLayout();
#endif
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
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

#elif defined(RAYLIB)
			ClearBackground(BLACK);
#else
			LogError("Attempted to clear canvas but either no rendering library is active or it has no defined actions");
#endif
		}

		void EndRenderingMarker()
		{
#if defined(RAYLIB)
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

		void DrawUploadedIndexBufferInstanced(const size_t& indicesStartByteOffset, const size_t& drawIndexCount, const size_t& drawInstanceCount)
		{
#if defined(OPENGL)
			glDrawElementsInstanced(GL_TRIANGLES, drawIndexCount, GL_UNSIGNED_INT, (const void*)indicesStartByteOffset, drawInstanceCount);
#endif
		}
	}
}