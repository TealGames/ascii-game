#pragma once
#include "Core/Rendering/RenderUnit.hpp"
#include "Core/Rendering/TextureController.hpp"
#include "ECS/Component/Types/World/PointLight3DComponent.hpp"
#include "Utils/Math/Quaternion.hpp"
#include "Core/Rendering/Model3d.hpp"
#include "Core/Rendering/Raytracer.hpp"
#include <cstdint>

class EngineState;
class UIHierarchy;
class DebugInfo;
class CommandConsole;
class EntityEditorUI;
class CameraPrecalculatedData;

namespace Rendering
{
    enum class RaytraceMode : std::uint8_t
    {
        CPU     = 0,
        GPU     = 1,
    };

    struct ExtraPointLightData
    {
        Quat m_GlobalRot = {};
    };

    struct BlockData
    {
        bool m_ViewBufferNeedsUpdate = false;
        bool m_LightBufferNeedsUpdate = false;
        ExtraPointLightData m_ExtraPointLightData[MAX_POINT_LIGHTS] = { };
        LightBlockData m_LightBlock = {};
        ViewerBlockData m_ViewerBlock;
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
        Skybox           = 7,
        UI               = 8
    };
    inline constexpr CoreShaderIntegralType CORE_SHADER_COUNT = 9;

    using IntegralRenderPassType = std::uint8_t;
    enum class RenderPassType : IntegralRenderPassType
    {
        None        = 0,
        Shadow      = 1,
        Geometry    = 2,
        Skybox      = 3,
        PostProcess = 4,
        UI          = 5,
        RayTrace    = 6,
    };
    inline constexpr IntegralRenderPassType TOTAL_PASS_TYPES = 6;

    struct RenderPassData
    {
        RenderPassType m_PassType = RenderPassType::None;
        FrameBuffer* m_FrameBuffer = nullptr;
        //std::vector<RenderBatch*> m_Batches = {};
        //Shader* m_Shader = nullptr;

        bool UsesDefaultFrameBuffer() const;
    };

    enum class RenderCallType : std::uint8_t
    {
        Sphere3d = 0,
        Box3d = 1,
        Plane3d = 2,
        PointLight3d = 3,
        DirectionLight3d = 4,
        Model3d = 5,
    };
    std::string ToString(const RenderCallType call);

    struct RenderCallInvocation
    {
        RenderCallType m_Type = RenderCallType::Model3d;
        Mat4 m_ModelMatrix = {};

        std::string ToString() const;
    };

    struct GeometryMetrics
    {
        std::uint32_t m_TotalVertices = 0;
        std::uint32_t m_TotalIndices = 0;
        std::uint32_t m_TotalInstances = 0;
        std::uint32_t m_TotalEmissiveObjects = 0;
        std::vector<RenderCallInvocation> m_RenderCallInvocations;
    };
   
    using BVHIndexTree = StaticBVHTree<IndexType>;
    using BVHTriangleTree = StaticBVHTree<IndexTriangle>;
    using BVHInstanceBoundsTreeStd430 = StaticBVHTree<InstanceBoundsData, STD_430_ALIGN>;
    using BVHInstanceBoundsTree = StaticBVHTree<InstanceBoundsData>;

    class GraphicsManager;
    class Renderer
    {
    private:
        //TODO: make flags if many bools here
        bool m_isInit;
        bool m_isRenderStalled;
        size_t m_framesSinceStart;
        size_t m_unmovingFrames;

        std::array<RenderPassData, TOTAL_PASS_TYPES> m_renderPassData;
        RenderPassType m_currentPass;

        const EngineState* m_engineState;
        GraphicsManager* m_graphicsManager;
        std::array<Shader*, CORE_SHADER_COUNT> m_coreShaders;

        Raytracer m_raytracer;
        BVHInstanceBoundsTreeStd430 m_tlasTreeAligned;
        //The per-instance bounds data which contains that instance's
        //mesh data transformed into world bounds to be used to create tlas tree
        //and is temporary/not submitted to the shader
        std::vector<InstanceBoundsData> m_instanceBoundsData;
        std::vector<BVHNodeStd430> m_blasTreesAligned;

        std::vector<InstanceMesh> m_instanceMeshes;
        std::vector<MaterialData> m_materialData;
        std::vector<Texture*> m_renderTextures;
        std::vector<Texture> m_cpuBufferTextures;
        std::vector<std::uint32_t> m_emissiveInstanceIndices;
        //std::unordered_map<BatchHash, size_t> m_hashToBatchIndex;
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
        Texture m_texRaytraceAccum0;
        Texture m_texRaytraceAccum1;
        Texture m_texUIOutput;
        RenderBuffer m_hdrDepthRenderBuffer;

        VertexLayout m_geometryVertexLayout;
        VertexLayout m_uiVertexLayout;
        RenderUnit<Vertex, Instance> m_geometryUnit;
        RenderUnit<VertexUI, InstanceUI> m_uiUnit;
        BufferController m_bufferController;

        UniformBuffer m_viewerUniformBuffer;
        UniformBuffer m_lightUniformBuffer;

        ShaderStorageBuffer m_vertexStorageBuffer;
        ShaderStorageBuffer m_indexStorageBuffer;
        ShaderStorageBuffer m_instanceStorageBuffer;
        ShaderStorageBuffer m_instanceMeshStorageBuffer;
        ShaderStorageBuffer m_materialStorageBuffer;
        ShaderStorageBuffer m_emissiveInstanceIndexStorageBuffer;
        ShaderStorageBuffer m_tlasTreeStorageBuffer;
        ShaderStorageBuffer m_blasTreesStorageBuffer;

        BlockData m_uniformData;
    public:
       
    private:
        std::uint8_t GenerateRuntimeMaterialId();
        void ResetRuntimeMaterialId();
        std::filesystem::path CreateRaytraceOutputPath();

        RenderBatch& CreateGeometryBatch(Shader& shader, Material& material, const Vertex* vertexArray, const size_t vertexSize,
            const IndexType* indexArray, const size_t indexSize, const Mat4& modelMatrix, const BVHTriangleTree* blasTree);
        void FinishGeometryBatch(RenderBatch& batch, const BVHTriangleTree* blasTree);
        RenderBatch* TryGetSameGeometryDrawBatch(const Shader& shader, const Material& material, std::uint32_t vertexCount);
        void AddGeometryCompleteInstanceToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material);
        Instance& AddGeometryInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material);
        void AddGeometryInstanceMeshBoundsData(const std::uint32_t& instanceIndex);

        RenderBatch& CreateUIBatch(Shader& shader, Texture* texture, const VertexUI* vertexArray, const size_t vertexSize,
            const IndexType* indexArray, const size_t indexSize, const HDRColor& color, const float depth, const Mat3& modelMatrix);
        InstanceUI& AddUIInstanceDataToBatch(RenderBatch& batch, const HDRColor& color, Texture* texture, const float depth, const Mat3& modelMatrix);
        RenderBatch* TryGetSameUIDrawBatch(const Shader& shader, const Texture* texture, std::uint32_t vertexCount);

        MaterialData* CreateRuntimeMaterial(const Material& material);
        void ConstructBLASTree(BVHTriangleTree& tree, const size_t indexStart, const size_t indexSize);
        void ConstructTLASTree(const bool writeToTlasSSBO);
        int GetEnqueuedTextureIndex(Texture* texture);

        void FlushBatches();
        void DrawIndexedInstancedBatch(RenderBatch& batch, VertexLayout& bindLayout);
        void SetViewUniformBuffer(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix);
        void SetViewUniformBuffer(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix, 
            const Vec3& forwardDir, const Vec3& rightDir, const Vec3& upDir, const float yFov);
        void UpdateLightAndViewerBlock(const bool updateUniformBuffers);

        void ExecuteSkyboxPass(Texture& outputTexture, std::uint8_t* outDrawnAttachmentsMask);
        void ExecuteShadowPass();
        void ExecuteLightingAndGeometryPass(Texture& outputTexture, const SlotIndex* indices, 
            const std::uint8_t previousDrawnColorAttachmentsMask);
        Texture& ExecuteRayTracing(Texture* overrideOutputTexture);
        Texture& ExecuteRayTracingGPU(Texture& inputTex, Texture& outputTex, Texture* overrideOutputTexture);
        Texture& ExecuteRayTracingCPU(Texture& inputTex, Texture& outputTex, Texture* overrideOutputTexture);
        Texture& ExecuteForwardRendering();
        void ExecuteUIPass(Texture& textureInputOutput);
        void ExecutePostProcessPass(Texture& noPPColorOutputTex);

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

        void AddExistingMeshCall(Model3d& model, Shader& shader, Material& material, const Mat4& modelMatrix);

        //PRECONDITION: material must have a non-null albedo texture
        void AddCallBox3DMulti(Shader& shader, Material& material, const Mat4& modelMatrix);
        void AddCallBox3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix);

        //PRECONDITION: material must have a non-null albedo texture
        void AddCallSphere3DMulti(Shader& shader, Material& material, const Mat4& modelMatrix);
        void AddCallSphere3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix);
        void AddCallPlane3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix, const Vec2& textureRepeats);

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
        bool IntersectsBVH(Ray3D ray, const Vertex* outHitVertex);
        bool IsValidBVH();
        void AddBVHTreeBoundsWireframe();

        void AddCallBox3D(Material* material, const Mat4& modelMatrix);
        void AddCallSphere3D(Material* material, const Mat4& modelMatrix);
        void AddCallSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Quat& rotation);
        void AddCallTextureSphere3D(Material* material, const Mat4& modelMatrix);
        void AddCallTextureSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Quat& rotation);

        void AddCallTextureBox3D(Material* material, const Mat4& modelMatrix);
        void AddCallPlane3D(Material* material, const Mat4& modelMatrix, const Vec2& textureRepeats = Vec2::One());
        void AddCallRect2D(const HDRColor& color, Texture* texture, const float depth, const Mat3& modelMatrix);
        //void AddCallText(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Color color);

        void AddCallModel(Model3d& model, const Mat4& modelMatrix);
        void AddCallMesh(const ModelMesh& mesh, Material& material, const Mat4& modelMatrix);

        //void AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Color color);
        //void AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Color color);

        void AddCallPointLight(const WorldPosition3D& worldPos, const Quat& rotation, const float radius, const HDRColor& color);
        void SetDirectionalLight(const Vec3& dir, const HDRColor& color);
        void ClearDirectionalLight();

        //void AddCallPoints(PrimitiveType primitiveType, const WorldPosition3D* positions, const size_t& size);
        void AddCallAABBWifreframe(const Mat4& modelMatrix, const HDRColor& color, const float lineThickness);
        void AddCallAABBWifreframe(const AABB3D& aabb, const Quat& rotation, const HDRColor& color, const float lineThickness);

        void RenderBuffer();

        std::string ToStringBVH() const;
        std::string ToStringInstances() const;
        std::string ToStringMetrics() const;
        std::string ToStringAll() const;
    };
}