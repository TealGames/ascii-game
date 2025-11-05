#pragma once 
#include "Core/Rendering/Material.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
	class Renderer2d
	{
	private:
	public:

	private:
	public:
		void AddCallRectangle2DMulti(Shader& shader, const Vec2& worldSize,
			const Mat4& modelMatrix, Material& material);

		void AddCallPolygon2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Color color);
		void AddCallCircle2D(const float radius, const Mat4& modelMatrix, const Color color);
		void AddCallRectangle2D(const Vec2& worldSize, const Mat4& modelMatrix, const Color& color);

		void AddCallTexture2D(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Color color);
	};
}
