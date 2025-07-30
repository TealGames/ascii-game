#pragma once
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "Core/Rendering/Shader/Shader.hpp"
#include "Core/Rendering/Buffers.hpp"

namespace Rendering
{
	class Texture;
	class Font;
	namespace Backend
	{
		void LoadBackend();
		void SetViewport(const int x, const int y, const int width, const int height);
		
		VertexBuffer CreateVertexBuffer(const void* vertexArray, const size_t& elementSize, const size_t& arraySize, const VertexAttributeAdvance advanceType);
		IndexBuffer CreateIndexBuffer(const IndexType* indexArray, const size_t elementCount);
		VertexLayout CreateVertexLayout();

		void BeginRenderingMarker();
		void ClearBackground();
		void EndRenderingMarker();

		void DrawCircle(const WorldPosition3D& pos, const float radius, const Utils::Color color);
		void DrawRectangle(const WorldPosition3D& pos, const Vec2& size, const Utils::Color color);
		void DrawTexture(const WorldPosition3D& destinationPos, const Vec2& destinationSize, const Vec2& sourcePos, const Vec2& sourceSize,
			const Texture& tex, const float rotation, const Utils::Color color);
		void DrawText(const WorldPosition3D& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

		void DrawLine(const WorldPosition3D& startPos, const WorldPosition3D& endPos, const float thickness, const Utils::Color color);
		void DrawRectangleLine(const WorldPosition3D& pos, const float thickness, const Vec2& size, const Utils::Color color);

		void DrawUploadedIndexBuffer(const size_t& indicesStartByteOffset, const size_t& drawIndexCount);
		void DrawUploadedIndexBufferInstaced(const size_t& indicesStartByteOffset, const size_t& drawIndexCount, const size_t& drawInstanceCount);
	}
}