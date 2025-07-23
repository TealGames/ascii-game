#pragma once
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/Color.hpp"

namespace Rendering
{
	class Texture;
	class Font;
	namespace Backend
	{
		void LoadBackend();
		void SetViewport(const int x, const int y, const int width, const int height);

		void BeginRenderingMarker();
		void ClearBackground();
		void EndRenderingMarker();

		void DrawCircle(const ScreenPosition& pos, const float radius, const Utils::Color color);
		void DrawRectangle(const ScreenPosition& pos, const Vec2& size, const Utils::Color color);
		void DrawTexture(const ScreenPosition& destinationPos, const Vec2& destinationSize, const Vec2& sourcePos, const Vec2& sourceSize, 
			const Texture& tex, const float rotation, const Utils::Color color);
		void DrawText(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

		void DrawLine(const ScreenPosition& startPos, const ScreenPosition& endPos, const float thickness, const Utils::Color color);
		void DrawRectangleLine(const ScreenPosition& pos, const float thickness, const Vec2& size, const Utils::Color color);
	}
}