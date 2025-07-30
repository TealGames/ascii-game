#pragma once
#include "Core/Rendering/RenderCall.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include <cstdint>

class EngineState;
class UIHierarchy;
class DebugInfo;
class CommandConsole;
class EntityEditorUI;

namespace Rendering
{
    struct Vertex
    {
        WorldPosition3D m_Pos;
    };
    struct InstanceData
    {
        Utils::Color m_Color;
        Mat4 m_ModelMatrix;
    };
    using VertexType = Vertex;
    using InstanceType = InstanceData;

    class Shader;
    struct RenderBatch
    {
        const Shader* m_Shader = nullptr;
        std::vector<Vertex> m_Vertices = {};
        std::vector<IndexType> m_VertexIndices = {};
        std::vector<InstanceData> m_InstanceData = {};
    };

    enum class BatchFlushType : std::uint8_t
    {
        StateChange     = 0,
        FrameEnd        = 1,
    };

    struct StaticFrameRenderData
    {
        bool m_UpdatedDataThisFrame = false;
        Mat4 m_ViewMatrix = {};
        Mat4 m_ProjectionMatrix = {};
    };

    class Renderer
    {
    private:
        const EngineState* m_engineState;
        struct StaticFrameRenderData m_staticRenderData;

        std::vector<RenderCall> m_renderCalls;
        std::vector<TextCallData> m_textData;
        std::vector<TextureCallData> m_textureData;

        const Shader* m_defaultShader;

        BatchFlushType m_flushType;
        std::vector<RenderBatch> m_batches;

        VertexLayout m_layout;
        BufferController m_bufferController;

        IndexBuffer m_indexBuffer;
        VertexBuffer m_vertexBuffer;
        VertexBuffer m_instancedBuffer;
    public:

    private:
        void AddVerticesToBatch(const Shader* shader, const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize);
        void FlushBatches();
    public:
        Renderer(const EngineState& engineState);
        void Init();

        void AddCircleCall(const WorldPosition3D& centerPos, const float radius, const Utils::Color color);
        void AddRectangleCall(const WorldPosition3D& topLeftPos, const Vec2& size, const Utils::Color color);
        void AddTextureCall(const WorldPosition3D& topLeftPos, const Texture& tex, const float rotation, const Vec2 scale, const Utils::Color color);
        void AddTextCall(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

        void AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Utils::Color color);
        void AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Utils::Color color);

        void PushCallsToBuffer(const std::vector<RenderCall>& calls);
        void MoveCallsToBuffer(std::vector<RenderCall>& calls);

        void RenderBuffer();
        void ClearCommandBuffers();
    };
}