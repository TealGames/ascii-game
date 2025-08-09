#pragma once
#include "Core/Rendering/RenderCall.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Core/Rendering/TextureController.hpp"
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
        UV m_UVPos;

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
        Texture* m_Texture = nullptr;
        std::vector<VertexType> m_Vertices = {};

        /// <summary>
        /// Since we use local indices for easier calcualtions
        /// we need a way to convert to global vertex index for a batch
        /// so we add offset to all indices added (offset is just size of 
        /// vertex count prior to the first model instance being added
        /// </summary>
        IndexType m_IndexOffset = 0;
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
        TextureController m_textureController;

        IndexBuffer m_indexBuffer;
        VertexBuffer m_vertexBuffer;
        VertexBuffer m_instancedBuffer;
    public:
       
    private:
        void BatchStateChangeCheck(const Shader* shader, Texture* texture);
        void AddVerticesToBatch(const Shader* shader, Texture* texture,
            const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize);
        void AddVertexToBatch(const Shader* shader, const Vertex& vertex);
        void AddIndexToBatch(const IndexType& index);
        void AddIndicesToBatch(const std::array<IndexType, 3>& arr);
        void AddInstanceDataToBatch(const Mat4& modelMatrix, const Utils::Color& color);

        void FlushBatches();

        const Shader* GetDefaultShader() const;
        const Shader* GetTextureShader() const;
        void FrameRenderDataUpdateCheck();
        StaticFrameRenderData& GetThisFrameRenderData();

        void AddRectangleCall2DMulti(const Shader* shader, Texture* texture, const Vec2& worldSize, 
            const Mat4& modelMatrix, const Utils::Color& color);
    public:
        Renderer(const EngineState& engineState);
        void Init();
        bool WasInit() const;

        void AddPolygonCall2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Utils::Color color);
        void AddCircleCall2D(const float radius, const Mat4& modelMatrix, const Utils::Color color);
        void AddRectangleCall2D(const Vec2& worldSize, const Mat4& modelMatrix, const Utils::Color& color);
        void AddTextureCall(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Utils::Color color);
        void AddTextCall(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

        void AddBoxCall3D(const Vec3& size, const Mat4& modelMatrix, const Utils::Color& color);
        void AddSphereCall3D(const float radius, const Mat4& modelMatrix, const Utils::Color color);

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