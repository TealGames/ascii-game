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
class CameraPrecalculatedData;

namespace Rendering
{
    struct Vertex
    {
        WorldPosition3D m_Pos;

        std::string ToString() const;
    };
    
    struct InstanceData
    {
        Vec4 m_Color;
        Mat4 m_ModelMatrix;

        std::string ToString() const;
    };
    using VertexType = Vertex;
    using InstanceType = InstanceData;

    class Shader;
    struct RenderBatch
    {
        const Shader* m_Shader = nullptr;
        std::vector<VertexType> m_Vertices = {};
        std::vector<IndexType> m_VertexIndices = {};
        std::vector<InstanceType> m_InstanceData = {};

        std::string ToString() const;
    };
  
    enum class BatchFlushType : std::uint8_t
    {
        StateChange     = 0,
        FrameEnd        = 1,
    };

    struct StaticFrameRenderData
    {
        bool m_UpdatedDataThisFrame = false;
        const CameraPrecalculatedData* m_CameraData = {};
    };

    class Renderer
    {
    private:
        bool m_isInit;

        const EngineState* m_engineState;
        StaticFrameRenderData m_staticRenderData;

        std::vector<RenderCall> m_renderCalls;
        std::vector<TextCallData> m_textData;
        std::vector<TextureCallData> m_textureData;

        BatchFlushType m_flushType;
        std::vector<RenderBatch> m_batches;

        VertexLayout m_layout;
        BufferController m_bufferController;

        IndexBuffer m_indexBuffer;
        VertexBuffer m_vertexBuffer;
        VertexBuffer m_instancedBuffer;
    public:
       
    private:
        void AddVerticesToBatch(const Shader* shader,
            const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize);
        void AddInstanceDataToBatch(const Mat4& modelMatrix, const Utils::Color& color);
        void FlushBatches();

        const Shader* GetDefaultShader() const;
        void FrameRenderDataUpdateCheck();
        StaticFrameRenderData& GetThisFrameRenderData();
    public:
        Renderer(const EngineState& engineState);
        void Init();
        bool WasInit() const;

        void AddCircleCall(const WorldPosition3D& centerPos, const float radius, const Utils::Color color);
        void AddRectangleCall2D(const WorldPosition3D& centerLocalPos, const Vec2& size, const Mat4& modelMatrix, const Utils::Color& color);
        void AddRectangleCall3D(const WorldPosition3D& centerLocalPos, const Vec3& size, const Mat4& modelMatrix, const Utils::Color& color);
        void AddTextureCall(const WorldPosition3D& topLeftPos, const Texture& tex, const float rotation, const Vec2 scale, const Utils::Color color);
        void AddTextCall(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

        void AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Utils::Color color);
        void AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Utils::Color color);

        void PushCallsToBuffer(const std::vector<RenderCall>& calls);
        void MoveCallsToBuffer(std::vector<RenderCall>& calls);

        void RenderBuffer();
        void ClearCommandBuffers();

        std::string ToStringBatches() const;
        std::string ToStringAll();
    };
}