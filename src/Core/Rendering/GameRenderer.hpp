#pragma once
#include "Core/Rendering/RenderCall.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/TextureController.hpp"
#include "Utils/Data/Matrix.hpp"
#include "Core/Rendering/Material.hpp"
#include "ECS/Component/Types/World/PointLight3DComponent.hpp"
#include "Utils/Data/Quaternion.hpp"
#include "Core/Rendering/Model3d.hpp"
#include <cstdint>

class EngineState;
class UIHierarchy;
class DebugInfo;
class CommandConsole;
class EntityEditorUI;
class CameraPrecalculatedData;

namespace Rendering
{
    using VertexType = Vertex;
    using InstanceType = InstanceData;

    struct PointLightData
    {
        WorldPosition3D m_Pos;
        //This is padding for vec3 since we can only have 2 or 4 floats
        float _padding0;
        Vec4 m_Color;
        float m_Radius;
        uint32_t m_ShadowMapIndex;
        //This is padding to round data to 16 byte alignment
        float _padding1[2];

        PointLightData();
        PointLightData(const WorldPosition3D& pos, const Vec4& color, const float radius);
    };
    struct DirectionalLightData
    {
        Vec3 m_Direction = {};
        float _padding0 = 0;
        Vec4 m_Color = {};

        DirectionalLightData();
        DirectionalLightData(const Vec3& dir, const Vec4& color);
    };
    constexpr size_t MAX_POINT_LIGHTS = 2;
    struct LightBlockData
    {
        DirectionalLightData m_DirLight;
        int m_PointLightsCount;
        float _padding[3];
        PointLightData m_PointLights[MAX_POINT_LIGHTS];
    };

    class Shader;
    struct RenderBatch
    {
        Shader* m_Shader = nullptr;
        Texture* m_Texture = nullptr;
        //bool m_removeAfterFlush = true;
        std::vector<VertexType> m_Vertices = {};

        /// <summary>
        /// Since we use local indices for easier calcualtions
        /// we need a way to convert to global vertex index for a batch
        /// so we add offset to all indices added (offset is just size of 
        /// vertex count prior to the first model instance being added
        /// </summary>
        //IndexType m_IndexOffset = 0;
        std::vector<IndexType> m_VertexIndices = {};
        std::vector<InstanceType> m_InstanceData = {};

        std::string ToString() const;
    };
    using BatchHash = std::uint64_t;
    using BatchIndex = std::uint8_t;
    constexpr BatchIndex INVALID_BATCH_INDEX = -1;
    struct BatchKey
    {
        std::uint16_t m_ShaderId;
        std::uint16_t m_TextureId;
        std::uint32_t m_VertexCount;
    };

   /* struct StaticFrameRenderData
    {
        bool m_UpdatedDataThisFrame = false;
        const CameraPrecalculatedData* m_CameraData = {};
    };*/

    struct ExtraPointLightData
    {
        Quat m_GlobalRot = {};
    };
    
    struct UniformBufferData
    {
        bool m_CameraUpdatedThisFrame = false;
        bool m_LightingUpdatedThisFrame = false;
        ExtraPointLightData m_ExtraPointLightData[MAX_POINT_LIGHTS] = { };
        LightBlockData m_LightBlock = {};
    };
    
    using CoreShaderIntegralType = std::uint8_t;
    enum class CoreShader : CoreShaderIntegralType
    {
        Default          = 0,
        ForwardRender    = 1,
        Shadow           = 2,
        Texture          = 3,
        PostProcess      = 4,
        GaussianBlur     = 5
    };
    inline constexpr CoreShaderIntegralType CORE_SHADER_COUNT = 6;

    using IntegralRenderPassType = std::uint8_t;
    enum class RenderPassType : IntegralRenderPassType
    {
        None        = 0,
        Shadow      = 1,
        Geometry    = 2,
        PostProcess = 3
    };
    inline constexpr IntegralRenderPassType TOTAL_PASS_TYPES = 3;

    struct RenderPassData
    {
        RenderPassType m_PassType = RenderPassType::None;
        FrameBuffer* m_FrameBuffer = nullptr;

        bool UsesDefaultFrameBuffer() const;
    };
   
    class Renderer
    {
    private:
        bool m_isInit;
        bool m_isRenderStalled;
        size_t m_framesSinceStart;
        size_t m_frameDrawCalls;

        std::array<RenderPassData, TOTAL_PASS_TYPES> m_renderPassData;
        RenderPassType m_currentPass;

        const EngineState* m_engineState;
        std::array<Shader*, CORE_SHADER_COUNT> m_coreShaders;
        //StaticFrameRenderData m_staticRenderData;
        UniformBufferData m_uniformData;

        std::vector<RenderCall> m_renderCalls;
        std::vector<TextCallData> m_textData;
        std::vector<TextureCallData> m_textureData;

        std::vector<RenderBatch> m_batches;
        std::unordered_map<BatchHash, size_t> m_hashToBatchIndex;

        VertexLayout m_layout;
        BufferController m_bufferController;
        TextureSlotController m_textureController;
        ImageSlotController m_imageController;
         
        FrameBuffer m_frameBuffer;
        FrameBuffer* m_boundFrameBuffer;
        TextureCube m_shadowMaps[MAX_POINT_LIGHTS];
        Texture m_hdrColorOutput;
        RenderBuffer m_hdrDepthRenderBuffer;

        IndexBuffer m_indexBuffer;
        VertexBuffer m_vertexBuffer;
        VertexBuffer m_instancedBuffer;
        UniformBuffer m_viewerUniformBuffer;
        UniformBuffer m_lightUniformBuffer;
    public:
       
    private:
        RenderBatch* TryGetBatch(const Shader* shader, const Texture* texture, std::uint32_t vertexCount);
        size_t CalculateBatchHash(const Shader* shader, const Texture* texture, std::uint32_t totalVertices) const;
        size_t CalculateBatchHash(const RenderBatch& batch) const;
        RenderBatch& CreateBatch(Shader* shader, Texture* texture,
            const Vertex* vertexArray, const size_t vertexSize, const IndexType* indexArray, const size_t indicesSize,
            const Mat4& modelMatrix, const Utils::Color& color, const bool isFinished);
        void FinishBatch(RenderBatch& batch);
        void AddVertexToBatch(RenderBatch& batch, const Vertex& vertex);
        void AddIndicesToBatch(RenderBatch& batch, const std::array<IndexType, 3>& arr);
        void AddInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Utils::Color& color);

        void FlushBatches();
        void RenderStartActions() const;
        void SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix);
        void DrawBatch(RenderBatch& batch);
        void ExecuteShadowPass();
        void ExecutePostProcessPass();
        void ExecuteLightingAndGeometryPass(const SlotIndex* indices);

        void ApplyBlurInPlace(Texture& inputTexture, Texture& outputTexture);
        void RenderEndActions();

        /*
        Shader* GetDefaultShader() const;
        Shader* GetTextureShader() const;
        Shader* GetForwardRenderShader() const;
        */

        Shader* GetCoreShader(const CoreShader shader);
        Shader* GetBaseShader();
        Shader* GetBaseTextureShader();
        Texture* GetBaseAlbedo();
        Texture* GetMaterialAlbedo(Material& material);

        RenderPassType GetCurrentPass() const;
        RenderPassData& GetCurrentPassData();
        RenderPassData& GetPassDataMutable(const RenderPassType type);
        void UpdatePassRenderState(const RenderPassType pass);

        //void FrameRenderDataUpdateCheck();
        //StaticFrameRenderData& GetThisFrameRenderData();

        void AddCallRectangle2DMulti(Shader* shader, Texture* texture, const Vec2& worldSize, 
            const Mat4& modelMatrix, const Utils::Color& color);
        void AddCallBox3DMulti(Shader* shader, Texture* texture, const Vec3& worldSize,
            const Mat4& modelMatrix, const Utils::Color& color);
        void AddCallSphere3DMulti(Shader* shader, Texture* texture, const float radius, 
            const Mat4& modelMatrix, const Utils::Color color);

        /// <summary>
        /// Will calculate the length, width and height of a cube in terms of pixels
        /// relative to the texture size based on its world size. 
        /// Note: world size expects size along x, y, z axes (and returns in the same order)
        /// </summary>
        /// <param name="worldPos"></param>
        /// <param name="textureSize"></param>
        /// <returns></returns>
        Vec3Int CalculateFaceSizeForTexture(const WorldPosition3D& worldSize, const Vec2Int textureSize);

    public:
        Renderer(const EngineState& engineState);
        void Init();
        bool WasInit() const;

        void InitCoreShaders();

        void AddCallPolygon2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Utils::Color color);
        void AddCallCircle2D(const float radius, const Mat4& modelMatrix, const Utils::Color color);
        void AddCallRectangle2D(const Vec2& worldSize, const Mat4& modelMatrix, const Utils::Color& color);

        void AddCallBox3D(const Vec3& size, const Mat4& modelMatrix, const Utils::Color& color);
        void AddCallSphere3D(const float radius, const Mat4& modelMatrix, const Utils::Color color);

        void AddCallTexture2D(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Utils::Color color);
        void AddCallTextureSphere3D(const float radius, Texture& tex, const Mat4& modelMatrix, const Utils::Color color);
        void AddCallTextureBox3D(const Vec3& size, Material& material, const Mat4& modelMatrix);
        void AddCallText(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

        void AddCallModel(Model3d& model, const Mat4& modelMatrix);

        void AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Utils::Color color);
        void AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Utils::Color color);

        void AddCallPointLight(const WorldPosition3D& worldPos, const Quat& worldRot, const float radius, const Utils::Color color);
        void AddCallDirectionalLight(const Vec3& dir, const Utils::Color color);

        void PushCallsToBuffer(const std::vector<RenderCall>& calls);
        void MoveCallsToBuffer(std::vector<RenderCall>& calls);

        void RenderBuffer();
        void ClearCommandBuffers();

        std::string ToStringBatches() const;
        std::string ToStringAll();
    };
}