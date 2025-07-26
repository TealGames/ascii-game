#include "Core/Rendering/RenderingBackend.hpp"
#include "StaticGlobals.hpp"
#include "Core/Analyzation/Debug.hpp"

#ifdef OPENGL
#include "Utils/OpenGlUtils.hpp"
#endif

#ifdef GLFW
#include "GLFW/glfw3.h"
#endif

#ifdef RAYLIB
#include "raylib.h"
#include "Utils/RaylibUtils.hpp"
#endif 


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
		}

		void SetViewport(const int x, const int y, const int width, const int height)
		{
#if defined(OPENGL)
			glViewport(x, y, width, height);
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

		void DrawCircle(const ScreenPosition& pos, const float radius, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawCircle(pos.m_X, pos.m_Y, radius, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawRectangle(const ScreenPosition& pos, const Vec2& size, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawRectangle(pos.m_X, pos.m_Y, size.m_X, size.m_Y, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawTexture(const ScreenPosition& destinationPos, const Vec2& destinationSize, const Vec2& sourcePos, const Vec2& sourceSize,
			const Texture& tex, const float rotation, const Utils::Color color)
		{
#if defined(RAYLIB)
			//TODO: there needs to be a way to get raylib texture from Texture
			DrawTexturePro(Texture2D(), Rectangle{sourcePos.m_X, sourcePos.m_Y, sourceSize.m_X, sourceSize.m_Y},
				Rectangle{destinationPos.m_X, destinationPos.m_Y, destinationSize.m_X, destinationSize.m_Y}, { 0, 0 }, rotation, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawText(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
		{
#if defined(RAYLIB)
			//Note: the text seems to flicker less when we put text on integer boundaries 
			//TODO: convert font to rayib font
			DrawTextEx(::Font(), text, RaylibUtils::ToRaylibVector(Vec2Int(pos.m_X, pos.m_Y)), size, spacing, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawLine(const ScreenPosition& startPos, const ScreenPosition& endPos, const float thickness, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawLineEx(RaylibUtils::ToRaylibVector(startPos), RaylibUtils::ToRaylibVector(endPos), thickness, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawRectangleLine(const ScreenPosition& pos, const float thickness, const Vec2& size, const Utils::Color color)
		{
#if defined(RAYLIB)
			DrawRectangleLinesEx(Rectangle{pos.m_X, pos.m_Y, size.m_X, size.m_Y }, thickness, RaylibUtils::ToRaylibColor(color));
#endif
		}

		void DrawBatch(const Shader* shader, const Vertex* vertices, const size_t vertexSize, const IndexType* indices, const size_t indexSize)
		{
#if defined(OPENGL)

#endif
		}
	}
}