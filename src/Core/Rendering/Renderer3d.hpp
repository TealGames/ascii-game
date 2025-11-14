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
    using InstanceType = Instance;

    struct PointLightData
    {
        WorldPosition3D m_Pos;
        //This is padding for vec3 since we can only have 2 or 4 floats
        float _padding0;
        Color m_Color;
        float m_Radius;
        uint32_t m_ShadowMapIndex;
        //This is padding to round data to 16 byte alignment
        float _padding1[2];

        PointLightData();
        PointLightData(const WorldPosition3D& pos, const Color& color, const float radius);
    };
    struct DirectionalLightData
    {
        Vec3 m_Direction = {};
        float _padding0 = 0;
        Color m_Color = {};

        DirectionalLightData();
        DirectionalLightData(const Vec3& dir, const Color& color);
    };
    constexpr size_t MAX_POINT_LIGHTS = 2;
    struct LightBlockData
    {
        DirectionalLightData m_DirLight;
        int m_PointLightsCount;
        float _padding[3];
        PointLightData m_PointLights[MAX_POINT_LIGHTS];
    };

    //NOTE: must be aligned to std::430 (members and struct at 16 byte alignment)
    constexpr int INVALID_TEXTURE_INDEX = -1;
    struct MaterialData
    {
        Color m_BaseColor;
        Color m_EmissiveColor;
        float m_Alpha;
        float m_Metallic;
        float m_Roughness;
        int m_AlbedoIndex;
        //float _padding;

        MaterialData();
        MaterialData(const Material& material, const int albedoIndx);

        std::string ToString() const;
    };

    class Shader;
    struct RenderBatch
    {
        Shader* m_Shader = nullptr;
        Texture* m_Texture = nullptr;

        size_t m_VertexStartIndex = -1;
        size_t m_VertexCount = 0;

        size_t m_IndicesStartIndex = -1;
        size_t m_IndicesCount = 0;

        size_t m_InstanceStartIndex = -1;
        size_t m_InstanceCount = 0;
        //bool m_removeAfterFlush = true;
        //std::vector<VertexType> m_Vertices = {};

        /// <summary>
        /// Since we use local indices for easier calcualtions
        /// we need a way to convert to global vertex index for a batch
        /// so we add offset to all indices added (offset is just size of 
        /// vertex count prior to the first model instance being added
        /// </summary>
        //IndexType m_IndexOffset = 0;
        //std::vector<IndexType> m_VertexIndices = {};
        //std::vector<InstanceType> m_InstanceData = {};

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
        GaussianBlur     = 5,
        RayTrace         = 6,
        SkyboxConverted  = 7,
    };
    inline constexpr CoreShaderIntegralType CORE_SHADER_COUNT = 8;

    using IntegralRenderPassType = std::uint8_t;
    enum class RenderPassType : IntegralRenderPassType
    {
        None        = 0,
        Shadow      = 1,
        Geometry    = 2,
        PostProcess = 3,
        RayTrace    = 4,
    };
    inline constexpr IntegralRenderPassType TOTAL_PASS_TYPES = 4;

    struct RenderPassData
    {
        RenderPassType m_PassType = RenderPassType::None;
        FrameBuffer* m_FrameBuffer = nullptr;
        //Shader* m_Shader = nullptr;

        bool UsesDefaultFrameBuffer() const;
    };

    struct GeometryMetrics
    {
        std::uint32_t m_TotalVertices = 0;
        std::uint32_t m_TotalIndices = 0;
        std::uint32_t m_TotalInstances = 0;
        std::uint32_t m_TotalEmissiveObjects = 0;
    };
   
    class GraphicsManager;
    class Renderer
    {
    private:
        bool m_isInit;
        bool m_isRenderStalled;
        size_t m_framesSinceStart;
        size_t m_unmovingFrames;

        std::array<RenderPassData, TOTAL_PASS_TYPES> m_renderPassData;
        RenderPassType m_currentPass;

        const EngineState* m_engineState;
        GraphicsManager* m_graphicsManager;
        std::array<Shader*, CORE_SHADER_COUNT> m_coreShaders;
        //StaticFrameRenderData m_staticRenderData

        //TODO: the cpu side buffers should probabbly be fixed arrays
        std::vector<RenderBatch> m_batches;
        std::vector<VertexType> m_vertices;
        std::vector<IndexType> m_vertexIndices;
        std::vector<InstanceType> m_instances;
        std::vector<InstanceMesh> m_instanceMeshes;
        std::vector<MaterialData> m_materialData;
        std::vector<Texture*> m_bindQueuedTextures;
        std::vector<std::uint32_t> m_emissiveInstanceIndices;
        std::unordered_map<BatchHash, size_t> m_hashToBatchIndex;
        std::unordered_map<String16, std::uint32_t> m_cachedMaterials;
        std::uint8_t m_runtimeMaterialId;
        GeometryMetrics m_frameGeometryMetrics;

        TextureSlotController m_textureController;
        ImageSlotController m_imageController;
         
        FrameBuffer* m_boundFrameBuffer;
        Shader* m_boundShader;

        FrameBuffer m_frameBuffer;
        TextureCube m_shadowMaps[MAX_POINT_LIGHTS];
        Texture* m_skybox;
        //The io texture is used for scenarios when we need a secondary texture
        //for input and/or output to prevent writing/reading of same texture
        Texture m_ioTexture;
        Texture m_hdrColorOutput;
        Texture m_brightnessOutput;
        RenderBuffer m_hdrDepthRenderBuffer;

        VertexLayout m_vertexLayout;
        BufferController m_bufferController;

        IndexBuffer m_indexBuffer;
        VertexBuffer m_vertexBuffer;
        VertexBuffer m_instancedBuffer;
        UniformBuffer m_viewerUniformBuffer;
        UniformBuffer m_lightUniformBuffer;

        ShaderStorageBuffer m_vertexStorageBuffer;
        ShaderStorageBuffer m_indexStorageBuffer;
        ShaderStorageBuffer m_instanceStorageBuffer;
        ShaderStorageBuffer m_instanceMeshStorageBuffer;
        ShaderStorageBuffer m_materialStorageBuffer;
        ShaderStorageBuffer m_emissiveInstanceIndexStorageBuffer;

        UniformBufferData m_uniformData;
    public:
       
    private:
        std::uint8_t GenerateRuntimeMaterialId();
        void ResetRuntimeMaterialId();

        RenderBatch* TryGetBatch(const Shader& shader, const Texture& texture, std::uint32_t vertexCount);
        size_t CalculateBatchHash(const Shader& shader, const Texture& texture, std::uint32_t totalVertices) const;
        size_t CalculateBatchHash(const RenderBatch& batch) const;
        RenderBatch& CreateBatch(Shader& shader, Material& material, const Vertex* vertexArray, const size_t vertexSize,
            const IndexType* indexArray, const size_t indicesSize, const Mat4& modelMatrix, const bool isFinished);
        RenderBatch* TryGetSameDrawBatch(const Shader& shader, const Material& material, std::uint32_t vertexCount);
        void AddCompleteInstanceToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material);

        MaterialData* CreateRuntimeMaterial(const Material& material);
        void FinishBatch(RenderBatch& batch);
        void AddVertexToBatch(RenderBatch& batch, const Vertex& vertex);
        void AddVerticesToBatch(RenderBatch& batch, const Vertex* vertexArray, const size_t vertexSize);
        void AddIndicesToBatch(RenderBatch& batch, const std::array<IndexType, 3>& arr);
        void AddIndicesToBatch(RenderBatch& batch, const IndexType* indexArray, const size_t indicesSize);
        void AddInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material);
        void AddMeshInstanceToBatch(RenderBatch& batch);
        int GetEnqueuedTextureIndex(Texture* texture);
        void ClearQueuedTextures();

        void FlushBatches();
        void RenderStartActions() const;
        void SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix);
        void SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix, 
            const Vec3& forwardDir, const Vec3& rightDir, const Vec3& upDir, const float yFov);
        void DrawBatch(RenderBatch& batch);
        void ExecuteShadowPass();
        void ExecuteLightingAndGeometryPass(const SlotIndex* indices);
        void ExecuteRayTracing();
        void ExecuteForwardRendering();
        void ExecutePostProcessPass();

        /// <summary>
        /// Applies blur to the input texture DIRECTLY where output texture is only an intermediary
        /// between doing horizontal and vertical blur pass
        /// 
        /// Gaussian blur process:
        /// -> Applies blur in radial area around pixel by taking less weight of pixels as you move farther away from center pixel
        /// -> This implementation has radius of 5 (so 4 pixels from starting pixel is distance traveled)
        /// 
        /// Strength settings:
        /// -> 1.33 is the max blur strength, >1.3 makes no change to blur strength
        /// </summary>
        /// <param name="inputTexture"></param>
        /// <param name="outputTexture"></param>
        /// <param name="strength"></param>
        void ApplyBlurInPlace(Texture& inputTexture, Texture& outputTexture, const float strength);
        void RenderEndActions();

        Shader& GetCoreShader(const CoreShader shader);
        Shader& GetBaseShader();
        Shader& GetBaseTextureShader();
        void BindShader(Shader& shader);
        void UnbindActiveShader();
        void BindFrameBuffer(FrameBuffer* buffer);
        void UnbindActiveFrameBuffer();

        Texture& GetDefaultAlbedo();
        Texture& GetMaterialAlbedoOrDefault(Material& material);
        void SetMaterialAlbedoIfNull(Material& material);
        Material& GetDefaultMaterial();
        Material& GetMaterialOrDefault(Material* material);

        RenderPassType GetCurrentPass() const;
        RenderPassData& GetCurrentPassData();
        RenderPassData& GetPassDataMutable(const RenderPassType type);
        void UpdatePassRenderState(const RenderPassType pass);

        //PRECONDITION: material must have a non-null albedo texture
        void AddCallBox3DMulti(Shader& shader, Material& material, const Vec3& worldSize,
            const Mat4& modelMatrix);

        //PRECONDITION: material must have a non-null albedo texture
        void AddCallSphere3DMulti(Shader& shader, Material& material, const float radius,
            const Mat4& modelMatrix);

        /// <summary>
        /// Will calculate the length, width and height of a cube in terms of pixels
        /// relative to the texture size based on its world size. 
        /// Note: world size expects size along x, y, z axes (and returns in the same order)
        /// </summary>
        /// <param name="worldPos"></param>
        /// <param name="textureSize"></param>
        /// <returns></returns>
        Vec3Int CalculateFaceSizeForTexture(const WorldPosition3D& worldSize, const Vec2Int textureSize);
        void CalculateCubeMapMatrices(const Vec3 pos, const float nearDistance, const float farDistance, 
            std::array<Mat4, 6>& outViewMatrices, Mat4& outProjMatrix);

    public:
        Renderer(const EngineState& engineState);
        void Init();
        bool WasInit() const;

        void InitCoreShaders();
        
        void SetSkybox(Texture* texture);

        void AddCallBox3D(Material* material, const Vec3& size, const Mat4& modelMatrix);
        void AddCallSphere3D(Material* material, const float radius, const Mat4& modelMatrix);
        void AddCallTextureSphere3D(Material* material, const float radius, const Mat4& modelMatrix);
        void AddCallTextureBox3D(Material* material, const Vec3& size, const Mat4& modelMatrix);
        void AddCallPlane3D(Material* material, const Vec2& size, const Mat4& modelMatrix);
        //void AddCallText(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Color color);

        void AddCallModel(Model3d& model, const Mat4& modelMatrix);

        //void AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Color color);
        //void AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Color color);

        void AddCallPointLight(const WorldPosition3D& worldPos, const Quat& worldRot, const float radius, const Color color);
        void AddCallDirectionalLight(const Vec3& dir, const Color color);

        void RenderBuffer();

        std::string ToStringBatches() const;
        std::string ToStringAll();
    };
}