#include "Core/Rendering/Renderer2d.hpp"

namespace Rendering
{
    void Renderer2d::AddCallRectangle2DMulti(Shader& shader, const Vec2& worldSize,
        const Mat4& modelMatrix, Material& material)
    {
        //TODO: implement
        LogError(std::format("2D draw call is not supported right now"));
        return;

        /*
        constexpr size_t VERTEX_COUNT = 4;
        constexpr size_t INDEX_COUNT = 6;

        RenderBatch* sameStatebatch = TryGetSameDrawBatch(shader, material, INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddInstanceDataToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }

        const WorldPosition3D halfSize = Vec3(worldSize / 2, 0);
        //Start with top right vertex, then bottom right, then bottom left, top left
        const Vertex vertices[VERTEX_COUNT] = { Vertex(halfSize, UV(1, 1)), Vertex(halfSize * Vec3(1, -1, 0), UV(1, 0)),
                                     Vertex(halfSize * Vec3(-1, -1, 0), UV(0 ,0)), Vertex(halfSize * Vec3(-1, 1, 0), UV(0, 1)) };


        IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2 };
        CreateBatch(shader, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, &material, true);
        */
    }

    void Renderer2d::AddCallPolygon2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Color color)
    {
        //TODO: implement
        LogError(std::format("2D draw call is not supported right now"));
        return;

        /*
        //TODO: the polygon and circle calls 2d should instead be textures that are drawon on quads to allow for batching
        const float angleStep = 2 * std::numbers::pi / sides;
        const size_t vertexCount = sides + 1;
        const size_t indexCount = sides * 3;

        //m_renderCalls.emplace_back(CircleCall{ centerPos, radius, color });
        if (Utils::ApproximateEqualsF(color.m_A, MAX_FLOAT_COLOR_CHANNEL))
        {
            //Note: the total number of vertices is horizontal-1 * vertical * 6 since we create square
            //for every 2 pairs going downward, thus needing to exlude the final horizontal row
            // + vertical * 3 (north pole)+ vertical*3 (south pole) since each vertical connects with top
            RenderBatch* sameStatebatch = TryGetBatch(GetCoreShader(CoreShader::Default), nullptr, indexCount);

            if (sameStatebatch != nullptr)
            {
                AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                return;
            }
        }

        Vertex* vertices = (Vertex*)alloca(sizeof(Vertex) * vertexCount);
        vertices[0] = {};
        for (size_t i = 1; i < vertexCount; i++)
        {
            vertices[i] = {};
            vertices[i].m_LocalPos = WorldPosition3D(std::cosf(i * angleStep) * radius, std::sinf(i * angleStep) * radius, 0);
        }

        IndexType* indices = (IndexType*)alloca(sizeof(IndexType) * indexCount);
        for (size_t i = 0; i < sides; i++)
        {
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            //The last vertex index needs to wrap around to start with index 1
            indices[i * 3 + 2] = i < sides - 1 ? i + 2 : 1;
        }
        CreateBatch(GetCoreShader(CoreShader::Default), nullptr, vertices, vertexCount, indices, indexCount, modelMatrix, color, true);
        */
    }
    void Renderer2d::AddCallCircle2D(const float radius, const Mat4& modelMatrix, const Color color)
    {
        LogError(std::format("2D draw call is not supported right now"));
        return;

        //AddCallPolygon2D(radius, CIRCLE_SIDE_COUNT, modelMatrix, color);
    }
    void Renderer2d::AddCallRectangle2D(const Vec2& worldSize, const Mat4& modelMatrix, const Color& color)
    {
        LogError(std::format("2D draw call is not supported right now"));
        return;

        //AddCallRectangle2DMulti(GetCoreShader(CoreShader::Default), nullptr, worldSize, modelMatrix, color);
    }

    void Renderer2d::AddCallTexture2D(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Color color)
    {
        LogError(std::format("2D draw call is not supported right now"));
        return;

        //AddCallRectangle2DMulti(GetBaseTextureShader(), &tex, worldSize, modelMatrix, color);
    }
}