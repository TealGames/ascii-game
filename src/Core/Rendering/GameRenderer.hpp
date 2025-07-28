#pragma once
#include "Core/Rendering/RenderCall.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include <cstdint>

class UIHierarchy;
class DebugInfo;
class CommandConsole;
class EntityEditorUI;

namespace Rendering
{
    class Shader;
    struct RenderBatch
    {
        const Shader* m_Shader = nullptr;
        std::vector<Vertex> m_Vertices = {};
        std::vector<IndexType> m_VertexIndices = {};
    };

    enum class BatchFlushType : std::uint8_t
    {
        StateChange     = 0,
        FrameEnd        = 1,
    };

    class Renderer
    {
    private:
        std::vector<RenderCall> m_renderCalls;
        std::vector<TextCallData> m_textData;
        std::vector<TextureCallData> m_textureData;

        const Shader* m_defaultShader;

        BatchFlushType m_flushType;
        std::vector<RenderBatch> m_batches;
        size_t m_currentBatchIndex;
    public:

    private:
        void CreateBatches();
        void AddVerticesToBatch(const Shader* shader, const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize);
        void FlushBatches();
    public:
        Renderer();
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