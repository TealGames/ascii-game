//NOT USED
#include "pch.hpp"
#include <optional>
#include <queue>
#include "StaticGlobals.hpp"

#include "Core/Rendering/Renderer3d.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Utils/Debug.hpp"
#include "Core/EngineState.hpp"
#include "Core/Camera/CameraController.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Math/PlatformMath.hpp"
#include "Core/Window/WindowManager.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Utils/MathAdvanced.hpp"
#include "Utils/Data/ColorConstants.hpp"
#include "UnitTests.hpp"

namespace Rendering
{
    /// <summary>
    /// If true, all verticies must be present at the start before first frame update
    /// </summary>
    constexpr bool DO_STATIC_GEOMETRY = true;
    constexpr bool USE_CACHED_SHAPE_ASSETS = true;

    constexpr bool DO_RAYTRACING = true;
    constexpr std::uint32_t MAX_RAYTRACE_BOUNCES = 5;

    constexpr bool DO_VISUALIZE_BVH_BOUNDS = false;
    constexpr float BVH_BOUNDS_LINE_THICKNESS = 1;
    constexpr Color BVH_BOUNDS_COLOR = COLOR_GREEN;
    constexpr Color BVH_BOUNDS_LEAF_COLOR = COLOR_RED;

    constexpr bool DO_LIGHTING = true;
    constexpr bool DRAW_LIGHT_AREAS = false;
    constexpr bool USE_LIGHT_COLOR_FOR_RANGE = true;
    constexpr Color LIGHT_AREA_COLOR = {255, 255, 255, 255};
    
    constexpr bool DO_SHADOWS = false;
    constexpr Vec2Int SHADOW_MAP_SIZE = {256, 256};
    constexpr Vec2Int SKYBOX_MAP_SIZE = {512, 512};
    constexpr float SHADOW_NEAR_DISTANCE = 0.001;
    constexpr float SHADOW_FAR_DISTANCE = 1000;

    constexpr bool DO_POST_PROCESS = true;
    constexpr bool DO_HDR = true;
    constexpr bool DO_BLOOM = false;
    /// <summary>
    /// The luminance threshold needed for object to have bloom,
    /// lower the value, the more objects have bloom
    /// </summary>
    constexpr float BLOOM_THRESHOLD = 0.5;
    
    constexpr bool DO_EXPOSURE = true;
    /// <summary>
    /// Multiplier applied to final color
    /// </summary>
    constexpr float EXPOSURE = 1;

    constexpr size_t NO_RENDER_FRAME_COUNT_LIMIT = 0;
    constexpr size_t RENDER_FRAMES_COUNT = NO_RENDER_FRAME_COUNT_LIMIT;
    constexpr LogType STALL_LOG_TYPE = LogType::Warning;

    constexpr size_t INSTANCE_MAX_COUNT = 16;
    constexpr size_t INDEX_MAX_COUNT = 20000;
    constexpr size_t VERTEX_MAX_COUNT = 10000;
    constexpr size_t MATERIAL_MAX_COUNT = 10;
    constexpr size_t TEXTURE_MAX_COUNT = 5;
    constexpr size_t BLAS_NODE_MAX_COUNT = 8000;
    constexpr size_t TLAS_NODE_MAX_COUNT = 8000;
    constexpr size_t CIRCLE_SIDE_COUNT = 12;

    static const char* CORE_SHADER_NAMES[CORE_SHADER_COUNT] = { 
        "default", "forward_render", "shadow", "texture", 
        "post_process", "gaussian_blur", "ray_tracer", "skybox"};

    constexpr const char* VIEW_MATRIX_UNIFORM_NAME = "uViewMatrix";
    constexpr const char* PROJ_MATRIX_UNIFORM_NAME = "uProjectionMatrix";
    constexpr const char* TEXTURE_UNIFORM_NAME = "uAlbedo";
    constexpr const char* SHADOW_MAP_UNIFORM_NAME = "uShadowMaps";
    constexpr const char* HDR_TEXTURE_UNIFORM_NAME = "uHdrTexture";
    constexpr const char* SCREEN_SIZE_UNIFORM_NAME = "uScreenSize";

    constexpr const char* DO_BLOOM_UNIFORM_NAME = "uDoBloom";
    constexpr const char* BRIGHTNESS_TEXTURE_UNIFORM_NAME = "uBrightnessTexture";
    constexpr const char* BRIGHTNESS_IMAGE_UNIFORM_NAME = BRIGHTNESS_TEXTURE_UNIFORM_NAME;
    constexpr const char* BLOOM_THRESHOLD_UNIFORM_NAME = "uBloomThreshold";
    constexpr const char* EXPOSURE_UNIFORM_NAME = "uExposure";

    constexpr const char* VIEWER_UNIFORM_BLOCK_NAME = "ViewerBlock";
    constexpr const char* LIGHT_UNIFORM_BLOCK_NAME = "LightsBlock";
    constexpr const char* VERTEX_SSBO_BLOCK_NAME = "Vertices";
    constexpr const char* INDEX_SSBO_BLOCK_NAME = "Indices";
    constexpr const char* INSTANCE_SSBO_BLOCK_NAME = "Instances";
    constexpr const char* INSTANCE_MESHES_SSBO_BLOCK_NAME = "InstanceMeshes";
    constexpr const char* MATERIAL_SSBO_BLOCK_NAME = "Materials";
    constexpr const char* LIGHT_INDICES_SSBO_BLOCK_NAME = "LightIndices";
    constexpr const char* BLAS_TREES_SSBO_BLOCK_NAME = "BLASTrees";
    constexpr const char* TLAS_TREE_SSBO_BLOCK_NAME = "TLASTree";

    //For compute shaders
    constexpr const char* TEXTURES_UNIFORM_NAME = "uTextures";
    constexpr const char* SKYBOX_UNIFORM_NAME = "uSkybox";
    constexpr const char* HORIZONTAL_FLAG_UNIFORM_NAME = "uIsHorizontal";
    constexpr const char* INPUT_TEXTURE_UNIFORM_NAME = "uTextureInput";
    constexpr const char* OUTPUT_TEXTURE_UNIFORM_NAME = "uTextureOutput";
    constexpr const char* BLUR_WEIGHTS_UNIFORM_NAME = "uWeights";

    constexpr const char* RAY_TRACING_MAX_RAY_BOUNCES_UNIFORM_NAME = "uMaxBounces";
    constexpr const char* UNMOVING_FRAME_NUMBER_UNIFORM_NAME = "uUnmovingFrameCount";
    constexpr const char* EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME = "uEmissiveCount";
    constexpr const char* INSTANCE_COUNT_UNIFORM_NAME = "uInstanceCount";

    std::string ToString(const RenderCallType call)
    {
        if (call == RenderCallType::Box3d) return "Box3d";
        if (call == RenderCallType::Sphere3d) return "Sphere3d";
        if (call == RenderCallType::Plane3d) return "Plane3d";
        if (call == RenderCallType::PointLight3d) return "PointLight3d";
        if (call == RenderCallType::DirectionLight3d) return "DirLight3d";
        if (call == RenderCallType::Model3d) return "Model3d";

        LogError("Failed to convert render call type to string due to no actions defined");
        return "";
    }
    std::string RenderCallInvocation::ToString() const
    {
        return std::format("[RenderCallInvocation Type:{} ModelMat:{}]", 
            Rendering::ToString(m_Type), m_ModelMatrix.ToString());
    }

    PointLightData::PointLightData() : PointLightData({}, {}, 0) {}
    PointLightData::PointLightData(const WorldPosition3D& pos, const Color& color, const float radius)
        : m_Pos(pos), m_Color(color), m_Radius(radius), _padding0(0), _padding1{}, m_ShadowMapIndex(-1) {}

    DirectionalLightData::DirectionalLightData() : DirectionalLightData({}, {}) {}
    DirectionalLightData::DirectionalLightData(const Vec3& dir, const Color& color)
        : m_Direction(dir), m_Color(color), _padding0(0) {}

    MaterialData::MaterialData() : MaterialData(Material{}, INVALID_TEXTURE_INDEX) {}
    MaterialData::MaterialData(const Material& material, const int albedoIndx)
        : m_BaseColor(material.GetBaseColor()), m_Alpha(material.GetAlpha()), m_Metallic(material.GetMatallic()), 
        m_Roughness(material.GetRoughness()), m_EmissiveColor(material.GetEmissiveColor()), m_AlbedoIndex(albedoIndx) {}

    std::string MaterialData::ToString() const
    {
        return std::format("[MaterialData BaseColor:{} Alpha:{} EmissiveColor:{}]", 
            m_BaseColor.ToString(), m_Alpha, m_EmissiveColor.ToString());
    }

    WorldPosition3D InstanceBoundsData::GetCenter() const { return m_RootWorldBounds.GetCenter(); }
    AABB3D InstanceBoundsData::GetAABB() const { return m_RootWorldBounds; }
    std::string InstanceBoundsData::ToString() const 
    { 
        return std::format("[RootWorldBounds:{} InstIdx:{}]", 
            m_RootWorldBounds.ToString(), m_InstanceIndex); 
    }

    bool RenderPassData::UsesDefaultFrameBuffer() const
    {
        return m_FrameBuffer != nullptr;
    }

    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants
    Renderer::Renderer(const EngineState& engineState)
        : m_isInit(false), m_engineState(&engineState), m_uniformData(), m_skybox(),//m_staticRenderData(),
        m_graphicsManager(nullptr), m_frameGeometryMetrics(), m_runtimeMaterialId(),
        m_textureController(Backend::CreateTextureController()),
        m_imageController(Backend::CreateImageController()),
        m_instanceMeshes(), m_emissiveInstanceIndices(), m_geometryUnit(m_geometryVertexLayout),
        m_geometryVertexLayout(), m_bufferController(),
        m_unmovingFrames(0), m_isRenderStalled(false), m_framesSinceStart(0),
        m_frameBuffer(), m_shadowMaps(), m_hdrColorOutput(), m_hdrDepthRenderBuffer(), m_coreShaders({}),
        m_currentPass(RenderPassType::None), m_renderPassData({}), m_boundFrameBuffer(nullptr), m_boundShader(nullptr),
        m_viewerUniformBuffer(Backend::CreateUniformBuffer(VIEWER_UNIFORM_BLOCK_NAME)),
        m_lightUniformBuffer(Backend::CreateUniformBuffer(LIGHT_UNIFORM_BLOCK_NAME)),
        m_vertexStorageBuffer(Backend::CreateShaderStorageBuffer(VERTEX_SSBO_BLOCK_NAME)),
        m_indexStorageBuffer(Backend::CreateShaderStorageBuffer(INDEX_SSBO_BLOCK_NAME)),
        m_instanceStorageBuffer(Backend::CreateShaderStorageBuffer(INSTANCE_SSBO_BLOCK_NAME)),
        m_instanceMeshStorageBuffer(Backend::CreateShaderStorageBuffer(INSTANCE_MESHES_SSBO_BLOCK_NAME)),
        m_materialStorageBuffer(Backend::CreateShaderStorageBuffer(MATERIAL_SSBO_BLOCK_NAME)),
        m_emissiveInstanceIndexStorageBuffer(Backend::CreateShaderStorageBuffer(LIGHT_INDICES_SSBO_BLOCK_NAME)),
        m_tlasTreeStorageBuffer(Backend::CreateShaderStorageBuffer(TLAS_TREE_SSBO_BLOCK_NAME)),
        m_blasTreesStorageBuffer(Backend::CreateShaderStorageBuffer(BLAS_TREES_SSBO_BLOCK_NAME))
    {
        RenderPassType passType = RenderPassType::None;
        for (size_t i = 0; i < TOTAL_PASS_TYPES; i++)
        {
            //NOTE: we add 1 to cover none type
            passType = static_cast<RenderPassType>(i+1);
            RenderPassData& data = m_renderPassData[i];

            data.m_PassType = passType;
            if (passType == RenderPassType::Shadow)
            {
                data.m_FrameBuffer = &m_frameBuffer;
            }
            else if (passType == RenderPassType::Geometry && DO_POST_PROCESS)
            {
                data.m_FrameBuffer = &m_frameBuffer;
            }
            //NOTE: if we use custom framebuffer for geometry, we must use custom framebuffer for 
            //skybox to ensure it is drawn in the same texture as geometry (otherwise default frame
            //buffer will be enabled when Post Process is drawn which covers whole screen with texture)
            else if (passType == RenderPassType::Skybox && DO_POST_PROCESS)
            {
                data.m_FrameBuffer = &m_frameBuffer;
            }
        }
        UpdatePassRenderState(RenderPassType::None);
    }

    void Renderer::Init()
    {
        m_graphicsManager = m_engineState->m_GraphicsContext.m_GraphicsManager;

        //We reserve one for current batch, but also keep it as vector for future in case we do rendering in one go
        //m_batches.reserve(1);
        m_geometryVertexLayout = Backend::CreateVertexLayout();
        m_geometryVertexLayout.BindActive();
        m_geometryUnit.Init(VERTEX_MAX_COUNT, INDEX_MAX_COUNT, INSTANCE_MAX_COUNT);

        const VertexLayoutBindIndex vertexBindIndex = m_bufferController.AddVertexBuffer(&m_geometryVertexLayout, 
            &m_geometryUnit.m_VertexBufferHandle, &m_geometryUnit.m_IndexBufferHandle);
        std::vector<VertexAttribute> vertexAttributes =
        {
            VertexAttribute(0, 3, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_LocalPos)),
            VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_UVPos)),
            VertexAttribute(2, 3, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_Normal)),
        };
        m_geometryVertexLayout.AddAttributes(vertexBindIndex, vertexAttributes);

        const VertexLayoutBindIndex instancedBindIndex = m_bufferController.AddVertexBuffer(&m_geometryVertexLayout, &m_geometryUnit.m_InstanceBufferHandle, nullptr);
        std::vector<VertexAttribute> instancedAttributes =
        {
            VertexAttribute(3, 1, VertexAttributeBaseType::UnsignedInteger, false, offsetof(Instance, m_MaterialIndex)),
            VertexAttribute(4, 1, VertexAttributeBaseType::UnsignedInteger, false, offsetof(Instance, m_MeshIndex)),
        };
        m_geometryVertexLayout.AddAttributes(instancedBindIndex, instancedAttributes);
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 5, false, sizeof(Vec4), offsetof(Instance, m_ModelMatrix));
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 9, false, sizeof(Vec4), offsetof(Instance, m_InverseModelMatrix));
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(3, 3), instancedBindIndex, 13, false, sizeof(Vec3), offsetof(Instance, m_NormalModelMatrix));

        

        if (DO_RAYTRACING) m_instanceMeshes.reserve(INSTANCE_MAX_COUNT);

        //TODO: it is a little redudant to add buffer to graphics manager and buffer controller what if they were merged into one?
        m_graphicsManager->AddShaderBuffer(&m_lightUniformBuffer);
        m_graphicsManager->AddShaderBuffer(&m_viewerUniformBuffer);

        m_bufferController.AddShaderBuffer(&m_lightUniformBuffer);
        m_bufferController.AddShaderBuffer(&m_viewerUniformBuffer);

        //NOTE: the following below should always be done
        m_graphicsManager->AddShaderBuffer(&m_vertexStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_indexStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_instanceStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_instanceMeshStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_materialStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_emissiveInstanceIndexStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_tlasTreeStorageBuffer);
        m_graphicsManager->AddShaderBuffer(&m_blasTreesStorageBuffer);

        m_bufferController.AddShaderBuffer(&m_vertexStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_indexStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_instanceStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_instanceMeshStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_materialStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_emissiveInstanceIndexStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_tlasTreeStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_blasTreesStorageBuffer);

        m_frameBuffer = Backend::CreateFrameBuffer();
        //TODO: right now all shadows have same resolution -> this might be a light component setting
        if (DO_SHADOWS)
        {
            m_engineState->m_GraphicsContext.m_GraphicsManager->AddShaderGlobalDefine("DO_SHADOWS");

            for (auto& map : m_shadowMaps) 
                map = CreateTextureCube(SHADOW_MAP_SIZE, TexelStorageType::Depth24);
        }

        if (DO_HDR)
        {
            const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
            m_hdrColorOutput = CreateTexture(nullptr, windowSize, TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            m_brightnessOutput = CreateTexture(nullptr, windowSize, TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            m_ioTexture = CreateTexture(nullptr, windowSize, TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            m_hdrDepthRenderBuffer = Backend::CreateRenderBuffer(TexelStorageType::Depth24, windowSize);
        }

        m_graphicsManager->AddShaderGlobalDefine("MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS));
        m_graphicsManager->AddShaderGlobalDefine("VERTEX_MAX_COUNT " + std::to_string(VERTEX_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("INDEX_MAX_COUNT " + std::to_string(INDEX_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("INSTANCE_MAX_COUNT " + std::to_string(INSTANCE_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("MATERIAL_MAX_COUNT " + std::to_string(MATERIAL_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("TEXTURE_MAX_COUNT " + std::to_string(TEXTURE_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("BLAS_NODE_MAX_COUNT " + std::to_string(BLAS_NODE_MAX_COUNT));
        m_graphicsManager->AddShaderGlobalDefine("TLAS_NODE_MAX_COUNT " + std::to_string(TLAS_NODE_MAX_COUNT));

        m_isInit = true;
    }
    bool Renderer::WasInit() const
    {
        return m_isInit;
    }

    void Renderer::WriteGeometryVertexDataToSSBOs()
    {
        m_vertexStorageBuffer.WriteData(0, m_geometryUnit.GetVertexCount() *sizeof(Vertex), m_geometryUnit.GetVertexMemPointer());
        m_indexStorageBuffer.WriteData(0, m_geometryUnit.GetIndexCount() * sizeof(IndexType), m_geometryUnit.GetIndexMemPointer());
        m_instanceStorageBuffer.WriteData(0, m_geometryUnit.GetInstanceCount() * sizeof(Instance), m_geometryUnit.GetInstanceMemPointer());
        /*LogWarning(std::format("Instance mesh buffer elements:{} write:{} bytes:{}", m_instanceMeshStorageBuffer.GetAllocatedByteSize() / sizeof(InstanceMesh),
            m_instanceMeshes.size(), m_instanceMeshes.size() * sizeof(InstanceMesh)));*/
        m_instanceMeshStorageBuffer.WriteData(0, m_instanceMeshes.size() * sizeof(InstanceMesh), &m_instanceMeshes[0]);
        const std::uint32_t emissiveMaterialCount = m_emissiveInstanceIndices.size();
        if (emissiveMaterialCount > 0)
        {
            m_emissiveInstanceIndexStorageBuffer.WriteData(0,
                emissiveMaterialCount * sizeof(std::uint32_t), &m_emissiveInstanceIndices[0]);
        }

        m_blasTreesStorageBuffer.WriteData(0, sizeof(BVHFlatNode) * m_blasTrees.size(), &m_blasTrees[0]);
    }

    void Renderer::InitCoreShaders()
    {
        for (size_t i = 0; i < CORE_SHADER_COUNT; i++)
        {
            m_coreShaders[i] = m_graphicsManager->TryGetShaderMutable(CORE_SHADER_NAMES[i]);
            if (m_coreShaders[i] == nullptr)
            {
                LogError(std::format("Core shader at index:{} could not be retrieved by name:{} All Resources: {}", 
                    i, CORE_SHADER_NAMES[i], m_graphicsManager->ToStringLoadedResources()));
                return;
            }
        }

        m_graphicsManager->SetUniform(UniformDataType::Float, BLOOM_THRESHOLD_UNIFORM_NAME, &BLOOM_THRESHOLD);
        m_graphicsManager->SetUniform(UniformDataType::Bool, DO_BLOOM_UNIFORM_NAME, &DO_BLOOM);

        constexpr float exposure = DO_EXPOSURE ? EXPOSURE : 1;
        m_graphicsManager->SetUniform(UniformDataType::Float, EXPOSURE_UNIFORM_NAME, &exposure);

        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
        m_graphicsManager->SetUniform(UniformDataType::IVector2, SCREEN_SIZE_UNIFORM_NAME, &windowSize);
        m_graphicsManager->SetUniform(UniformDataType::Uint, RAY_TRACING_MAX_RAY_BOUNCES_UNIFORM_NAME, &MAX_RAYTRACE_BOUNCES);

       /* m_viewerUniformBuffer.AllocateFromShaderUniformBlock(*GetCoreShader(CoreShader::ForwardRender));
        m_lightUniformBuffer.AllocateFromShaderUniformBlock(*GetCoreShader(CoreShader::ForwardRender));*/

        //m_engineState->m_GraphicsContext.m_GraphicsManager->SetUniform(UniformDataType::Bool, SHADOW_TOGGLE_UNIFORM_NAME, &DO_SHADOWS);
        //const std::string_view defines[] = {"DO_SHADOWS"};
        //GetCoreShader(CoreShader::ForwardRender)->TryCreateProgram({ defines, 1 });
    }

    std::uint8_t Renderer::GenerateRuntimeMaterialId()
    {
        return m_runtimeMaterialId++;
    }
    void Renderer::ResetRuntimeMaterialId()
    {
        m_runtimeMaterialId = 0;
    }

    Shader& Renderer::GetCoreShader(const CoreShader shader)
    {
        if (m_coreShaders[0] == nullptr) InitCoreShaders();
        return *m_coreShaders[static_cast<CoreShaderIntegralType>(shader)];
    }
    Shader& Renderer::GetBaseShader()
    {
        if (DO_LIGHTING) return GetCoreShader(CoreShader::ForwardRender);
        return GetCoreShader(CoreShader::Default);
    }
    Shader& Renderer::GetBaseTextureShader()
    {
        if (DO_LIGHTING) return GetCoreShader(CoreShader::ForwardRender);
        return GetCoreShader(CoreShader::Default);
    }
    void Renderer::BindShader(Shader& shader)
    {
        shader.BindActive();
        m_boundShader = &shader;
    }
    void Renderer::UnbindActiveShader()
    {
        if (m_boundShader == nullptr)
            return;
        m_boundShader->UnbindActive();
        m_boundShader = nullptr;
    }
    void Renderer::BindFrameBuffer(FrameBuffer* buffer)
    {
        if (m_boundFrameBuffer != buffer)
        {
            if (m_boundFrameBuffer != nullptr)
                m_boundFrameBuffer->UnbindActive();
            if (buffer != nullptr)
                buffer->BindActive();

            m_boundFrameBuffer = buffer;
        }
        //Whether the new bound is the same as before or is compeltely different
        //we reset ouput
        if (m_boundFrameBuffer != nullptr)
            m_boundFrameBuffer->RemoveAllOutputs();
    }
    void Renderer::UnbindActiveFrameBuffer()
    {
        BindFrameBuffer(nullptr);
    }
    Texture& Renderer::GetDefaultAlbedo()
    {
        return *m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultAlbedoMutable();
    }
    Texture& Renderer::GetMaterialAlbedoOrDefault(Material& material)
    {
        if (material.m_Albedo == nullptr)
            return GetDefaultAlbedo();
        return *material.m_Albedo;
    }
    void Renderer::SetMaterialAlbedoIfNull(Material& material)
    {
        if (material.m_Albedo == nullptr)
            material.m_Albedo = &GetDefaultAlbedo();
    }
    Material& Renderer::GetDefaultMaterial()
    {
        return *m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultMaterialMutable();
    }
    Material& Renderer::GetMaterialOrDefault(Material* material)
    {
        Material* materialResult = material;
        if (materialResult == nullptr)
            materialResult = &GetDefaultMaterial();

        SetMaterialAlbedoIfNull(*materialResult);
        return *material;
    }
    RenderBatch* Renderer::TryGetSameGeometryDrawBatch(const Shader& shader, const Material& material, std::uint32_t vertexCount)
    {
        //TODO: also consider alpha of texture and then return nullptr if it has alpha != 255
        if (!Utils::ApproximateEqualsF(material.GetAlpha(), MAX_FLOAT_COLOR_CHANNEL))
            return nullptr;

        if (material.m_Albedo == nullptr)
        {
            LogError(std::format("Attempted to get same draw batch as args with "
                "shader:{} material:{} and vertexCount:{} but material has no albedo assigned", 
                shader.ToString(), material.ToString(), vertexCount));
            return nullptr;
        }
        return m_geometryUnit.TryGetBatch(shader, *material.m_Albedo, vertexCount);
    }

    RenderPassType Renderer::GetCurrentPass() const
    {
        return m_currentPass;
    }
    RenderPassData& Renderer::GetCurrentPassData()
    {
        if (m_currentPass == RenderPassType::None)
        {
            LogError(std::format("Attempted to get current pass data for current pass: NONE"));
            throw std::invalid_argument("Invalid pass state");
        }
        //NOTE: - 1 for none type
        return GetPassDataMutable(m_currentPass);
    }
    RenderPassData& Renderer::GetPassDataMutable(const RenderPassType type)
    {
        return m_renderPassData[static_cast<IntegralRenderPassType>(type) - 1];
    }
    void Renderer::UpdatePassRenderState(const RenderPassType pass)
    {
        m_currentPass = pass;
        //NOTE: we do not unbind bound framebuffer on none pass type in case 
        //the next frame we want to start with the last framebuffer bound in previous frame
        if (m_currentPass == RenderPassType::None)
            return;

        auto& currPassData = GetPassDataMutable(pass);
        BindFrameBuffer(currPassData.m_FrameBuffer);
    }

    RenderBatch& Renderer::CreateGeometryBatch(Shader& shader, Material& material,
        const Vertex* vertexArray, const size_t vertexSize, const IndexType* indexArray, const size_t indicesSize,
        const Mat4& modelMatrix, const BVHTriangleTree* blasTree)
    {
        SetMaterialAlbedoIfNull(material);

        RenderBatch& batch = m_geometryUnit.CreateBatch(shader, material.m_Albedo);
        if (vertexSize > 0 && vertexArray != nullptr)
        {
            m_geometryUnit.AddVerticesToBatch(batch, vertexArray, vertexSize);
        }
        if (indicesSize > 0 && indexArray != nullptr)
        {
            m_geometryUnit.AddIndicesToBatch(batch, indexArray, indicesSize);
        }
        
        AddGeometryInstanceDataToBatch(batch, modelMatrix, material);
        //NOTE: we wait until we have vertices and indices to be able to finish batch
        if (vertexArray != nullptr && indexArray != nullptr) FinishGeometryBatch(batch, blasTree);
        return batch;
    }
    Instance& Renderer::AddGeometryInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material)
    {
        Mat3 normalMatrix = modelMatrix.GetSlice<3, 3>();
        ENGINE_ASSERT(normalMatrix.Inverse(), "Attempted to add instance data to batch with model matrix:{} "
            "but 3x3 normal model matrix failed to inverse:{}", modelMatrix.ToString(), normalMatrix.ToString());

        auto cachedMaterialIt = m_cachedMaterials.find(material.m_Name);
        if (cachedMaterialIt == m_cachedMaterials.end())
        {
            m_materialData.emplace_back(MaterialData(material, GetEnqueuedTextureIndex(material.m_Albedo)));
            cachedMaterialIt = m_cachedMaterials.emplace(material.m_Name, m_materialData.size() - 1).first;
        }
        Instance& createdInstance = m_geometryUnit.AddInstanceDataToBatch(batch, 
            cachedMaterialIt->second, 0, modelMatrix, normalMatrix.Transpose());

        if (DO_RAYTRACING)
        {
            if (material.GetEmissiveColor().HasVisibleNonzeroRGB()) 
                m_emissiveInstanceIndices.push_back(m_geometryUnit.GetInstanceCount() - 1);
        }
        return createdInstance;
    }
    void Renderer::AddGeometryInstanceMeshBoundsData(const std::uint32_t& instanceIndex)
    {
        const Instance& instance = m_geometryUnit.m_CpuInstances[instanceIndex];
        //We get the root node of this instance's mesh blas tree (NODE: first index of interval is ROOT)
        const BVHFlatNode& blasTreeRootNode = m_blasTrees[m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval.m_StartIndex];
        m_instanceBoundsData.push_back(InstanceBoundsData(Utils::ApplyMatrixToAABB(blasTreeRootNode.GetAABB(), instance.m_ModelMatrix), instanceIndex));
    }
    void Renderer::FinishGeometryBatch(RenderBatch& batch, const BVHTriangleTree* blasTree)
    {
        m_geometryUnit.FinishBatch(batch);

        //TODO: technically it is bad to create tree + instance mesh for every time we finish batch
        //because we may have cases where some batch equality/hash paramter is different (though mesh is the same)
        //so we create new batch with identical mesh even if some things like alpha differ -> in that case we 
        //want to reuse the old mesh data so we need to setup map of some kind

        BVHTriangleTree constructedTree = {};
        //NOTE: if we do not provide a tree, we just construct one from the batch indices
        //and since we do not actually need tree, jsut the nodes to copy, it does not matter if it gets destroyed at the end of the scope
        if (blasTree == nullptr)
        {
            ConstructBLASTree(constructedTree, batch.m_IndicesStartIndex, batch.m_IndicesCount);
        }
        const BVHTriangleTree& batchBLASTree = (blasTree == nullptr) ? constructedTree : *blasTree;

        const size_t blasNodeIndexOffset = m_blasTrees.size();
        m_blasTrees.reserve(m_blasTrees.size() + batchBLASTree.Size());
        for (const auto& node : batchBLASTree.GetNodes())
        {
            m_blasTrees.push_back(node);
            //NOTE: since the blas tree nodes used LOCAL INDICES
            //into a given index array segment, and it thus needs to be adapted into the global index array by
            //getting an offset of BATCH START INDEX / 3 (because blas object node indices are TRIANGLE INDICES not any indices)
            if (m_blasTrees.back().IsLeaf()) m_blasTrees.back().m_ObjectStartIndex += batch.m_IndicesStartIndex / 3;
            //NOTE: since the indices are local in terms of the root node of the tree, we add the root nodes
            //distance from the start of the tree to all nodes to adjust it
            else
            {
                m_blasTrees.back().m_IndexChild0 += blasNodeIndexOffset;
                m_blasTrees.back().m_IndexChild1 += blasNodeIndexOffset;
            }
        }

        //NOTE: the only reason why we can use the last mesh's entry for this offset into blas trees
        //is because they are all contiguous in terms of placement into the blas trees vector and every mesh-> one tree, one tree-> one mesh
        const size_t nextTreeStartIndex = m_instanceMeshes.empty() ? 0 : m_instanceMeshes.back().m_BLASTreesInterval.GetEndIndex();
        //When we finish a batch now that we added the tree we can finish mesh data
        m_instanceMeshes.emplace_back(InstanceMesh(batch.m_IndicesStartIndex, 
            batch.m_IndicesCount, ArrayInterval(nextTreeStartIndex, batchBLASTree.Size())));
        
        /*if (m_instanceMeshes.back().m_BLASTreesInterval.m_Size == 1) 
            LogError(std::format("Created array interval: {} for tree:{}", batchBLASTree.Size(), batchBLASTree.ToString(BVHToStringType::NodeBounds)));*/

        m_geometryUnit.m_CpuInstances[batch.m_InstanceStartIndex].m_MeshIndex = m_instanceMeshes.size() - 1;
        AddGeometryInstanceMeshBoundsData(batch.m_InstanceStartIndex);
    }
    void Renderer::AddGeometryCompleteInstanceToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material)
    {
        if (batch.m_InstanceCount <= 0)
        {
            LogError(std::format("Attempted to add complete instance to batch before the batch was setup with its first instance"));
            return;
        }

        //NOTE: we do NOT add separate mesh instance on creation since those should only be done
        //on the first isntance created for a batch because all instances in a batch share same mesh
        Instance& createdInstance = AddGeometryInstanceDataToBatch(batch, modelMatrix, material);
        //If this is NOT the first instance to this batch it means the mesh for this batch must exist (NOTE: the
        //mesh for a whole batch is the same) and we can set it to a previous set instance OTHERWSIE we
        //will have to set the mesh index separately when the batch is fully finished
        createdInstance.m_MeshIndex = m_geometryUnit.m_CpuInstances[batch.m_InstanceStartIndex].m_MeshIndex;
        //The most recent added instance data is the one create world mesh bounds
        AddGeometryInstanceMeshBoundsData(batch.m_InstanceStartIndex + batch.m_InstanceCount);
    }

    void Renderer::ConstructBLASTree(BVHTriangleTree& tree, const size_t indexStart, const size_t indexSize)
    {
        Rendering::ConstructBVHFromIndices(tree, &m_geometryUnit.m_CpuIndices[indexStart],
            indexSize, m_geometryUnit.GetVertexMemPointer());
    }
    void Renderer::ConstructTLASTree()
    {
        if (m_instanceBoundsData.empty())
            return;

        //LogWarning(std::format("ALL instance bounds data: {}", Utils::ToStringIterable(m_instanceBoundsData)));
        m_tlasTree.Construct(&m_instanceBoundsData[0], m_instanceBoundsData.size(), true, 1, 
            BVHSplitAlgorithm::Midpoint, &InstanceBoundsData::GetAABB, &InstanceBoundsData::GetCenter, 
            [](const InstanceBoundsData* boundsPtr, const std::uint32_t* objectIndicesArr, const size_t boundsSize, int intendedStartIndex,
                int& outStartIndex, std::uint32_t& outObjectCount) -> void
            {
                //NOTE: the only reason we can do this is because we know there will be only one 
                //instance at the leaves
                outStartIndex = boundsPtr[intendedStartIndex].m_InstanceIndex;
                outObjectCount = boundsSize;
            });
        m_tlasTreeStorageBuffer.WriteData(0, sizeof(BVHFlatNode) * m_tlasTree.Size(), &m_tlasTree.GetRoot());

        /*LogWarning(std::format("ALL instance bounds data: {}", Utils::ToStringIterable(m_instanceBoundsData)));
        LogWarning(std::format("ALL render calls: {}", ToStringMetrics()));
        LogWarning(std::format("ALL INSTANCES:{}", ToStringInstances()));
        LogWarning(std::format("TLAS TREE {}\n", m_tlasTree.ToString(BVHToStringType::NodeBounds)));
        LogWarning(std::format("FULL TREE {}\n", ToStringBVH()));*/

        //TODO: isValidBVH gives us false negatives
        //ENGINE_ASSERT(IsValidBVH(), "After finishing TLAS tree construction full BVH is INVALID: {}", ToStringBVH());
    }
    int Renderer::GetEnqueuedTextureIndex(Texture* texture)
    {
        if (texture == nullptr)
        {
            return INVALID_TEXTURE_INDEX;
        }

        for (std::uint8_t i = 0; i < m_bindQueuedTextures.size(); i++)
        {
            if (m_bindQueuedTextures[i] == texture)
                return i;
        }

        if (m_bindQueuedTextures.size() >= TEXTURE_MAX_COUNT)
        {
            LogError(std::format("Attempted to enqueue texture for bind but reached max texture count: {}/{}",
                m_bindQueuedTextures.size(), TEXTURE_MAX_COUNT));
            return INVALID_TEXTURE_INDEX;
        }

        m_bindQueuedTextures.push_back(texture);
        return m_bindQueuedTextures.size() - 1;
    }
    void Renderer::ClearQueuedTextures()
    {
        m_bindQueuedTextures.clear();
    }

    MaterialData* Renderer::CreateRuntimeMaterial(const Material& material)
    {
        auto cachedMaterialIt = m_cachedMaterials.find(material.m_Name);
        if (cachedMaterialIt != m_cachedMaterials.end())
        {
            LogError(std::format("Attempted to create runtime material named:{} but one exists with that name", material.m_Name));
            return nullptr;
        }
        MaterialData* createdData = &(m_materialData.emplace_back(MaterialData(material, GetEnqueuedTextureIndex(material.m_Albedo))));
        m_cachedMaterials.emplace(material.m_Name, m_materialData.size() - 1);
        return createdData;
    }

    Vec3Int Renderer::CalculateFaceSizeForTexture(const WorldPosition3D& worldSize, const Vec2Int textureSize)
    {
        return Vec3Int(worldSize.m_X / (2 * worldSize.m_X + 2 * worldSize.m_Z) * textureSize.m_X, 
                       worldSize.m_Y / (worldSize.m_Y + 2 * worldSize.m_Z) * textureSize.m_Y, 
                       worldSize.m_Z/ (2 * worldSize.m_Z + 2 * worldSize.m_X) * textureSize.m_X);
    }
    void Renderer::CalculateCubeMapMatrices(const Vec3 pos, const float nearDistance, const float farDistance, 
        std::array<Mat4, 6>& outViewMatrices, Mat4& outProjMatrix)
    {
        outViewMatrices =
        {
            CalculateViewMatrix(pos, ENGINE_RIGHT_DIR,      -ENGINE_UP_DIR),
            CalculateViewMatrix(pos, -ENGINE_RIGHT_DIR,     -ENGINE_UP_DIR),
            CalculateViewMatrix(pos, ENGINE_UP_DIR,         ENGINE_FORWARD_DIR),
            CalculateViewMatrix(pos, -ENGINE_UP_DIR,        -ENGINE_FORWARD_DIR),
            CalculateViewMatrix(pos, ENGINE_FORWARD_DIR,    -ENGINE_UP_DIR),
            CalculateViewMatrix(pos, -ENGINE_FORWARD_DIR,   -ENGINE_UP_DIR)
        };
        outProjMatrix = PlatformMath::CalculatePlatformPerspectiveProjMatrix(Utils::ToRadians(90), 1, nearDistance, farDistance);
    }

    void Renderer::AddCallBox3DMulti(Shader& shader, Material& material, const Mat4& modelMatrix)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Box3d, modelMatrix);

        Model3d* boxModel = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMeshMutable(BasicMeshType::Cube);
        if (!USE_CACHED_SHAPE_ASSETS || boxModel == nullptr)
        {
            LogWarning(std::format("[Renderer3D]: Added custom-constructed box 3D render call due to NULL cube asset"));
            AddCallBox3DMultiConstructed(shader, material, modelMatrix);
            return;
        }

        ModelMesh& boxMesh = boxModel->m_Objects[0].m_Mesh;
        const size_t indexCount = boxMesh.m_Indices.size();
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, indexCount);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }
        CreateGeometryBatch(shader, material, &boxMesh.m_Vertices[0], boxMesh.m_Vertices.size(),
            &boxMesh.m_Indices[0], indexCount, modelMatrix, &boxMesh.m_BLASTree);
    }

    void Renderer::AddCallBox3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix)
    {
        constexpr size_t VERTEX_COUNT = 24;
        constexpr size_t INDEX_COUNT = 36;
        //NOTE: since opaque objects can get depth tested, we can cram as many of them as we want into
        //a batch as long as they have the same state, but for transparent objects
        //they need to have their own batch to ensure correct draw order
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }

        constexpr WorldPosition3D halfSize = { 0.5f, 0.5f, 0.5f };
        //ORDER is front face [top right, bottom right, bottom left, top left]
        //and then back face(looking from front face side) [top right, bottom right, bottom left, top left]
        /*
                  7------4
                 /|     /|
                3------0 |
                |  6 --| 5
                | /    |/
                2------1
        */
        const std::array<Vec3, 8> edges =
        {
            //FRONT FACE
            halfSize,
            halfSize * Vec3(1, -1, 1),
            halfSize * Vec3(-1, -1, 1),
            halfSize * Vec3(-1, 1, 1),
            //BACK FACE
            halfSize * Vec3(1, 1, -1),
            halfSize * Vec3(1, -1, -1),
            halfSize * Vec3(-1, -1, -1),
            halfSize * Vec3(-1, 1, -1)
        };

        //The size is in x, y, z axis 
        const Vec2 textureSize = material.m_Albedo->GetInfo().m_texelSize.AsFloat();
        //The size in texture pixel coords based on its world size
        const Vec3Int pixelSize = CalculateFaceSizeForTexture(Utils::ExtractScaleFromMatrix(modelMatrix),
            material.m_Albedo->GetInfo().m_texelSize);

        const Vec2 frontBackFaceSize = Vec2(pixelSize.m_X, pixelSize.m_Y) / textureSize;
        const Vec2 leftRightFaceSize = Vec2(pixelSize.m_Z, pixelSize.m_Y) / textureSize;
        const Vec2 topBottomFaceSize = Vec2(pixelSize.m_X, pixelSize.m_Z) / textureSize;
        std::array<Vec2, 14> uvs = {};
        //LEFT FACE (bottom left, top left, bottom right, top right)
        uvs[0] = Vec2(topBottomFaceSize.m_Y, 0);
        uvs[1] = uvs[0] + Vec2(0, leftRightFaceSize.m_Y);
        uvs[2] = uvs[0] + Vec2(leftRightFaceSize.m_X, 0);
        uvs[3] = uvs[0] + leftRightFaceSize;
        //FRONT FACE (bottom right, top right)
        uvs[4] = uvs[2] + Vec2(frontBackFaceSize.m_X, 0);
        uvs[5] = uvs[4] + Vec2(0, frontBackFaceSize.m_Y);
        //TOP FACE (top left, top right)
        uvs[6] = uvs[3] + Vec2(0, topBottomFaceSize.m_Y);
        uvs[7] = uvs[6] + Vec2(topBottomFaceSize.m_X, 0);
        //BOTTOM FACE (bottom left, bottom right)
        uvs[8] = uvs[2] - Vec2(0, topBottomFaceSize.m_Y);
        uvs[9] = uvs[8] + Vec2(topBottomFaceSize.m_X, 0);
        //RIGHT FACE (bottom right, top right)
        uvs[10] = uvs[4] + Vec2(leftRightFaceSize.m_X, 0);
        uvs[11] = uvs[4] + Vec2(0, leftRightFaceSize.m_Y);
        //BACK FACE (bottom right, top right)
        uvs[12] = uvs[10] + Vec2(frontBackFaceSize.m_X, 0);
        uvs[13] = uvs[12] + Vec2(0, frontBackFaceSize.m_Y);
        //UV INDEX PLACEMENT:
        /*
                    6-------7
                    |  TOP  |
            1-------3-------5-------11-------13
            | LEFT  | FRONT | RIGHT | BACK   |
            0-------2-------4-------10-------12
                    | BOTTOM|
                    8-------9
        */

        //FACE ORDER: Front, back, right, left, top, bottom
        std::array<Vec3, 6> normals =
        {
            -ENGINE_FORWARD_DIR,
            ENGINE_FORWARD_DIR,
            ENGINE_RIGHT_DIR,
            -ENGINE_RIGHT_DIR,
            ENGINE_UP_DIR,
            -ENGINE_UP_DIR
        };

        //FACE ORDER: Front, back, right, left, top, bottom
        //FACE EDGE ORDER: top right, bottom right, bottom left, top left
        //NOTE: all vertices are as if you are looking north with forward face in front of you
        Vertex vertices[VERTEX_COUNT] = {};
        //FRONT FACE (0, 1, 2, 3)
        vertices[0] = Vertex(edges[0], uvs[5], normals[0]);
        vertices[1] = Vertex(edges[1], uvs[4], normals[0]);
        vertices[2] = Vertex(edges[2], uvs[2], normals[0]);
        vertices[3] = Vertex(edges[3], uvs[3], normals[0]);
        //BACK FACE (4, 5, 6, 7)
        vertices[4] = Vertex(edges[4], uvs[11], normals[1]);
        vertices[5] = Vertex(edges[5], uvs[10], normals[1]);
        vertices[6] = Vertex(edges[6], uvs[12], normals[1]);
        vertices[7] = Vertex(edges[7], uvs[13], normals[1]);
        //RIGHT FACE (4, 5, 1, 0)
        vertices[8] = Vertex(edges[4], uvs[11], normals[2]);
        vertices[9] = Vertex(edges[5], uvs[10], normals[2]);
        vertices[10] = Vertex(edges[1], uvs[4], normals[2]);
        vertices[11] = Vertex(edges[0], uvs[5], normals[2]);
        //LEFT FACE (7, 6, 2, 3)
        vertices[12] = Vertex(edges[7], uvs[1], normals[3]);
        vertices[13] = Vertex(edges[6], uvs[0], normals[3]);
        vertices[14] = Vertex(edges[2], uvs[2], normals[3]);
        vertices[15] = Vertex(edges[3], uvs[3], normals[3]);
        //TOP FACE (4, 0, 3, 7)
        vertices[16] = Vertex(edges[4], uvs[7], normals[4]);
        vertices[17] = Vertex(edges[0], uvs[5], normals[4]);
        vertices[18] = Vertex(edges[3], uvs[3], normals[4]);
        vertices[19] = Vertex(edges[7], uvs[6], normals[4]);
        //BOTTOM FACE (5, 1, 2, 6)
        vertices[20] = Vertex(edges[5], uvs[9], normals[5]);
        vertices[21] = Vertex(edges[1], uvs[4], normals[5]);
        vertices[22] = Vertex(edges[2], uvs[8], normals[5]);
        vertices[23] = Vertex(edges[6], uvs[2], normals[5]);

        IndexType indices[INDEX_COUNT] =
        {
            /*FRONT FACE*/ 0,  1,  2,  0,  3,  2,
            /*BACK FACE*/  4,  5,  6,  4,  7,  6,
            /*RIGHT FACE*/ 8,  9,  10, 8,  11, 10,
            /*LEFT FACE*/  12, 13, 14, 12, 15, 14,
            /*TOP FACE*/   16, 17, 18, 16, 19, 18,
            /*BOTTOM FACE*/20, 21, 22, 20, 23, 22
        };

        const ModelObject& cubeMesh = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMesh(BasicMeshType::Cube)->m_Objects[0];
        CreateGeometryBatch(shader, material, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, nullptr);
    }

    void Renderer::AddCallSphere3DMulti(Shader& shader, Material& material, const Mat4& modelMatrix)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Sphere3d, modelMatrix);
        Model3d* sphereModel = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMeshMutable(BasicMeshType::Sphere);
        if (!USE_CACHED_SHAPE_ASSETS || sphereModel == nullptr)
        {
            LogWarning(std::format("[Renderer3D]: Added custom-constructed sphere 3D render call due to NULL sphere asset"));
            AddCallSphere3DMultiConstructed(shader, material, modelMatrix);
            return;
        }

        ModelMesh& sphereMesh = sphereModel->m_Objects[0].m_Mesh;
        const size_t indexCount = sphereMesh.m_Indices.size();
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, indexCount);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }
        CreateGeometryBatch(shader, material, &sphereMesh.m_Vertices[0], sphereMesh.m_Vertices.size(),
            &sphereMesh.m_Indices[0], indexCount, modelMatrix, &sphereMesh.m_BLASTree);
    }

    void Renderer::AddCallSphere3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix)
    {
        constexpr float RADIUS = 0.5f;
        //Note: this is the default UV method with longitudinal/"slices" (vertical) and latitudinal/"stacks" (horizontal) lines
        constexpr size_t HORIZONTAL_LINE_COUNT = 8;
        //Best shape is formed with 1.5 factor 
        //Note: the total number of vertices is horizontal-1 * vertical * 6 since we create square
        //for every 2 pairs going downward, thus needing to exlude the final horizontal row
        // + vertical * 3 (north pole)+ vertical*3 (south pole) since each vertical connects with top
        constexpr size_t VERTICAL_LINE_COUNT = HORIZONTAL_LINE_COUNT * 1.5f;
        constexpr size_t TOTAL_VERTEX_COUNT = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT + 2;
        constexpr size_t TOTAL_INDEX_COUNT = (HORIZONTAL_LINE_COUNT - 1) * VERTICAL_LINE_COUNT * 6 + VERTICAL_LINE_COUNT * 6;

        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }
        RenderBatch& batch = CreateGeometryBatch(shader, material, nullptr, TOTAL_VERTEX_COUNT, nullptr, TOTAL_INDEX_COUNT, modelMatrix, nullptr);
        //TODO: right now we do not have very good uv mapping for spheres-> need to increase verticies at poles for increased precision

        //Here we build all the other vertices and indices making sure to create 2 triangles of every quad possible on the sphere
        IndexType aIndex = 0, bIndex = 0;
        float u = 0, v = 0, phi = 0, theta = 0;
        IndexType nextV = 0, cIndex = 0, dIndex = 0;
        Vec3 pos = {};
        //The way this works is we go through every possible horizontal line and create vertex of curr horizontal line
        //as well as every possible vertical line. 
        //Note: we skip horizontal count+1 iteration because last row has no indices left to go lower
        //but vertical needs the one extra iteration to connect
        for (size_t hi = 0; hi < HORIZONTAL_LINE_COUNT; hi++)
        {
            //Note: we add one here because we do not want 0 angle horizontal line (since then all rings vertices are at the topmost point)
            //(and for symmetry, we do the same for bottom) so instead we just offset this range by 2
            v = (hi + 1) / (float)(HORIZONTAL_LINE_COUNT);
            phi = v * std::numbers::pi;

            for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
            {
                u = vi / (float)VERTICAL_LINE_COUNT;
                theta = u * 2 * std::numbers::pi;

                pos = Vec3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta));
                m_geometryUnit.AddVertexToBatch(batch, Vertex{ pos * RADIUS, UV(u, v), pos.GetNormalized() });

                if (hi == HORIZONTAL_LINE_COUNT - 1) continue;
                // For the layout imagine this shape (where A is current point)
                // Note: There are HORIZONTAL_COUNT + 1 total vertex indices per horizontal line (since we need it to
                // wrap around and connect with first point and VERTICAL_COUNT + 1 total vertex indices per vertical line
                //  A --- B
                //  |   / |
                //  |  /  |
                //  | /   |
                //  C --- D

                nextV = (vi + 1) % VERTICAL_LINE_COUNT;

                //We create them in order A, B, C, D
                aIndex = hi * (VERTICAL_LINE_COUNT)+vi;
                bIndex = hi * (VERTICAL_LINE_COUNT)+nextV;
                cIndex = (hi + 1) * (VERTICAL_LINE_COUNT)+vi;
                dIndex = (hi + 1) * (VERTICAL_LINE_COUNT)+nextV;

                m_geometryUnit.AddIndicesToBatch(batch, { aIndex, cIndex, bIndex });
                m_geometryUnit.AddIndicesToBatch(batch, { bIndex, cIndex, dIndex });
            }
        }

        //First we build the north pole vertex and create the indices
        //AddVertexToBatch(Vertex{ Vec3(0.0f, radius, 0.0f), UV(1.0f, 1.0f), ENGINE_UP_DIR });
        m_geometryUnit.AddVertexToBatch(batch, Vertex{ Vec3(0.0f, RADIUS, 0.0f), UV(1.0f, 1.0f), ENGINE_UP_DIR });
        IndexType poleIndex = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = vi;
            bIndex = (aIndex + 1) % VERTICAL_LINE_COUNT;
            m_geometryUnit.AddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        //Finally, we connect all the bottom latitude/row verticies to the south pole vertex
        //AddVertexToBatch(Vertex{ Vec3(0.0f, -1.5 * radius, 0.0f), UV(0.0f, 0.0f), -ENGINE_UP_DIR });
        m_geometryUnit.AddVertexToBatch(batch, Vertex{ Vec3(0.0f, -1.5 * RADIUS, 0.0f), UV(0.0f, 0.0f), -ENGINE_UP_DIR });
        poleIndex++;
        const IndexType bottomStartIndex = poleIndex - VERTICAL_LINE_COUNT - 1;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = bottomStartIndex + vi;
            if (vi < VERTICAL_LINE_COUNT - 1) bIndex = aIndex + 1;
            else bIndex = bottomStartIndex;
            m_geometryUnit.AddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        FinishGeometryBatch(batch, nullptr);
    }

    void Renderer::AddCallBox3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallSphere3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Quat& rotation)
    {
        AddCallSphere3DMulti(GetBaseShader(), GetMaterialOrDefault(material), 
            //NOTE: since sphere by default has diameter 1 (radius 0.5), ITS SCALE will ultiamtely be 2 * radius (or diameter)
            Utils::CalculateModelMatrix(nullptr, worldPos, Vec3(radius*2), rotation));
    }
    void Renderer::AddCallTextureSphere3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallTextureSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Quat& rotation)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), 
            Utils::CalculateModelMatrix(nullptr, worldPos, Vec3(radius), rotation));
    }

    void Renderer::AddCallTextureBox3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallPlane3D(Material* material, const Vec2& size, const Mat4& modelMatrix, 
        const Vec2& textureRepeats)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Plane3d, modelMatrix);

        constexpr size_t TOTAL_INDEX_COUNT = 6;
        constexpr size_t TOTAL_VERTEX_COUNT = 4;

        Shader& baseShader = GetBaseTextureShader();
        Material& baseMaterial = GetMaterialOrDefault(material);
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(baseShader, baseMaterial, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, baseMaterial);
            return;
        }

        Vertex vertices[TOTAL_VERTEX_COUNT] = {
            //Bottom Left, Bottom right, top right, top left
            Vertex(Vec3(-size.m_X/2, 0, -size.m_Y/2), { 0, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(size.m_X/2, 0, -size.m_Y/2), { textureRepeats.m_X, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(size.m_X/2, 0, size.m_Y/2), textureRepeats, ENGINE_UP_DIR),
            Vertex(Vec3(-size.m_X/2, 0, size.m_Y/2), { 0, textureRepeats.m_Y }, ENGINE_UP_DIR)
        };
        IndexType indices[TOTAL_INDEX_COUNT] = {0, 1, 2, 2, 3, 0};
        CreateGeometryBatch(baseShader, baseMaterial, vertices, TOTAL_VERTEX_COUNT, indices, TOTAL_INDEX_COUNT, modelMatrix, nullptr);
    }

    void Renderer::AddCallPointLight(const WorldPosition3D& worldPos, const Quat& worldRot, const float radius, const Color& color)
    {
        //LogWarning(std::format("Invoked light call with: {}", m_uniformData.m_LightBlock.m_PointLightsCount));
        if (m_uniformData.m_LightBlock.m_PointLightsCount >= MAX_POINT_LIGHTS)
        {
            LogError(std::format("Attempted to add point light call at:{} colored:{} "
                "but max points lights have been reached", worldPos.ToString(), color.ToString()));
            return;
        }

        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::PointLight3d, Mat4{});
        m_uniformData.m_LightBufferNeedsUpdate = true;

        std::uint32_t lightIndex = m_uniformData.m_LightBlock.m_PointLightsCount; 
        auto& pointlightData = m_uniformData.m_LightBlock.m_PointLights[lightIndex];
        pointlightData = PointLightData(worldPos, color, radius);
        pointlightData.m_ShadowMapIndex = lightIndex;
        m_uniformData.m_ExtraPointLightData[lightIndex] = ExtraPointLightData{worldRot};
        m_uniformData.m_LightBlock.m_PointLightsCount++;
        //LogWarning(std::format("Ended light call with: {}", m_uniformData.m_LightBlock.m_PointLightsCount));

        if (DRAW_LIGHT_AREAS)
        {
            Material lightMaterial = Material("[$LightArea:"+std::to_string(GenerateRuntimeMaterialId())+"]", &GetDefaultAlbedo(),
                USE_LIGHT_COLOR_FOR_RANGE ? color : LIGHT_AREA_COLOR);

            AddCallSphere3DMulti(GetCoreShader(CoreShader::Texture), lightMaterial,
                Utils::CalculateModelMatrix(nullptr, worldPos, std::min(0.1f * radius, 1.0f), Quat::Identity()));
        }
    }
    void Renderer::SetDirectionalLight(const Vec3& dir, const Color& color)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::DirectionLight3d, Mat4{});

        m_uniformData.m_LightBlock.m_DirLight = DirectionalLightData(dir, color);
        m_uniformData.m_LightBufferNeedsUpdate = true;
    }
    void Renderer::ClearDirectionalLight()
    {
        m_uniformData.m_LightBlock.m_DirLight = DirectionalLightData();
        m_uniformData.m_LightBufferNeedsUpdate = true;
    }

    void Renderer::AddCallModel(Model3d& model, const Mat4& modelMatrix)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Model3d, modelMatrix);

        ModelObject* obj = nullptr;
        //LogWarning(std::format("Found mesh group: {}", model.m_MeshGroups.size()));
        for (auto& meshGroup : model.m_ObjectGroups)
        {
            //LogWarning(std::format("Found mesh group indices : {}", meshGroup.m_MeshIndices.size()));
            for (auto& meshIndex : meshGroup.m_ObjectIndices)
            {
                obj = &(model.m_Objects[meshIndex]);

                AddCallMesh(obj->m_Mesh, obj->m_Material, modelMatrix * meshGroup.m_GlobalTransform);
            }
        }
    }
    void Renderer::AddCallMesh(const ModelMesh& mesh, Material& material, const Mat4& modelMatrix)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Model3d, modelMatrix);
        CreateGeometryBatch(GetBaseTextureShader(), material, &(mesh.m_Vertices[0]), mesh.m_Vertices.size(),
            &(mesh.m_Indices[0]), mesh.m_Indices.size(), modelMatrix, &mesh.m_BLASTree);
    }

    void Renderer::AddCallAABBWifreframe(const Mat4& modelMatrix, const Color& color, const float lineThickness)
    {
        constexpr size_t TOTAL_INDEX_COUNT = 36 * 4;
        constexpr size_t TOTAL_VERTEX_COUNT = 36 * 4;

        Shader& baseShader = GetBaseTextureShader();
        Material& baseMaterial = GetDefaultMaterial();
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(baseShader, baseMaterial, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, baseMaterial);
            return;
        }
        RenderBatch& batch = CreateGeometryBatch(baseShader, baseMaterial, nullptr, TOTAL_VERTEX_COUNT, nullptr, TOTAL_INDEX_COUNT, modelMatrix, nullptr);

        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const WorldPosition3D cameraPos = camera.GetTransform().GetWorldPos();
        Vec3 cameraDir = {};
        Vec3 cameraLineOrthogonal = {};

        const ModelMesh& cubeMesh = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMesh(BasicMeshType::Cube)->m_Objects[0].m_Mesh;
        WorldPosition3D vertex0 = {};
        WorldPosition3D vertex1 = {};
        for (IndexType i = 1; i < cubeMesh.m_Indices.size(); i++)
        {
            vertex0 = cubeMesh.m_Vertices[cubeMesh.m_Indices[i - 1]].m_LocalPos;
            vertex1 = cubeMesh.m_Vertices[cubeMesh.m_Indices[i]].m_LocalPos;
            cameraDir = (cameraPos - (vertex0 + vertex1) / 2).GetNormalized();
            cameraLineOrthogonal = CrossProduct(cameraDir, (vertex1 - vertex0).GetNormalized()).GetNormalized();

            //TODO: FINISH
        }
        FinishGeometryBatch(batch, nullptr);
    }
    void Renderer::AddCallAABBWifreframe(const AABB3D& aabb, const Quat& rotation, const Color& color, const float lineThickness)
    {
        AddCallAABBWifreframe(Utils::CalculateModelMatrix(nullptr, aabb.GetCenter(), aabb.GetSize(), rotation), color, lineThickness);
    }

    void Renderer::SetSkybox(Texture* texture)
    {
        m_skybox = texture;
    }
    void Renderer::AddBVHTreeBoundsWireframe()
    {
        for (const auto& node : m_tlasTree.GetNodes())
        {
            AddCallAABBWifreframe(node.GetAABB(), Quat::Identity(), 
                node.IsLeaf()? BVH_BOUNDS_LEAF_COLOR : BVH_BOUNDS_COLOR, BVH_BOUNDS_LINE_THICKNESS);
        }
    }
    bool Renderer::IntersectsBVH(const WorldPosition3D& rayWorldOrigin, Vec3 rayWorldDir, const Vertex* outHitVertex)
    {
        rayWorldDir = rayWorldDir.GetNormalized();

        Triangle* trianglePtr = reinterpret_cast<Triangle*>(&m_geometryUnit.m_CpuIndices[0]);
        return m_tlasTree.Intersects<Instance>(rayWorldOrigin, rayWorldDir, &m_geometryUnit.m_CpuInstances[0], nullptr, nullptr,
            [this, trianglePtr](const BVHFlatNode& node, const Instance& instance, 
                const WorldPosition3D& rayWorldOrigin, const Vec3& rayWorldDir, float* outTopHitDistance) -> bool
            {
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                const WorldPosition3D rayLocalOrigin = (instance.m_InverseModelMatrix * Vec4(rayWorldOrigin, 1)).GetXYZ();
                const WorldPosition3D rayLocalDir = (instance.m_InverseModelMatrix * Vec4(rayWorldDir, 0)).GetXYZ().GetNormalized();
                /*
                LogWarning(std::format("Inverse mat:{} rayO {}->{} rayDir {}->{}", instance.m_InverseModelMatrix.ToString(), 
                    rayOrigin.ToString(), rayLocalOrigin.ToString(), rayDir.ToString(), rayLocalDir.ToString()));
                LogWarning(std::format("Ray (LOCAL) {} -> {} reached blas level (LOCAL) area: {} SHOULD INTERSECT:{}", 
                    rayLocalOrigin.ToString(), rayLocalDir.ToString(),
                    Utils::ApplyMatrixToAABB(node.GetAABB(), instance.m_InverseModelMatrix).ToString(), 
                    Utils::RayIntersectsSphere(Vec3(), 0.2, rayLocalOrigin, rayLocalDir, nullptr)));
                    */
                return ::IntersectsBVH<Triangle>(rayLocalOrigin, rayLocalDir, &m_blasTrees[treeInterval.m_StartIndex],
                    treeInterval.m_Size, trianglePtr, nullptr, nullptr, nullptr,
                    [this, outTopHitDistance, &instance, rayWorldOrigin](const BVHFlatNode& node, const Triangle& triangle, const WorldPosition3D& rayLocalOrigin,
                        const Vec3& rayLocalDir, float* outBottomHitDistance) -> bool
                    {
                        /*
                        LogWarning(std::format("Ray {} -> {} reached vertex level with triangle: {} {} {}", rayLocalOrigin.ToString(), rayLocalDir.ToString(),
                            m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex0].m_LocalPos.ToString(),
                            m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex1].m_LocalPos.ToString(), m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex2].m_LocalPos.ToString()));
                        */

                        float outTEnter = 0;
                        const bool intersectsTriangle = Utils::RayIntersectsTriangle(m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex0].m_LocalPos,
                            m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex1].m_LocalPos, m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex2].m_LocalPos,
                            rayLocalOrigin, rayLocalDir, &outTEnter);
                        *outBottomHitDistance = (rayLocalDir * outTEnter).GetMagnitude();
                        const Vec3 worldHitPos = (instance.m_ModelMatrix * Vec4(rayLocalOrigin + rayLocalDir * outTEnter, 1)).GetXYZ();
                        *outTopHitDistance = (worldHitPos - rayWorldOrigin).GetMagnitude();
                        return intersectsTriangle;
                    });
            });
    }
    bool Renderer::IsValidBVH()
    {
        //NOTE: this should ONLY be called after TLAS tree has been constructed and some blas nodes are added
        Triangle* trianglePtr = reinterpret_cast<Triangle*>(&m_geometryUnit.m_CpuIndices[0]);
        return m_tlasTree.IsValid<Instance>(&m_geometryUnit.m_CpuInstances[0],
            //Override getBounds of BLAS leaf node primitives (InstanceType)
            [this](const Instance& instance) -> AABB3D
            {
                //NOTE: since we have change the object indices in the TLAS to be indices into instances,
                //and since each leaf in the TLAS has only 1 INSTANCE, we can just get the roots BLAS tree aabb
                //which should be the same as the object aabb
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                return Utils::ApplyMatrixToAABB(m_blasTrees[treeInterval.m_StartIndex].GetAABB(), instance.m_ModelMatrix);
            },
            //TLAS leaf successor is valid function
            [this, trianglePtr](const BVHFlatNode& leafNode) -> bool
            {
                //NOTE: the object indices of TLAS tree are indices into instances
                const Instance& instance = m_geometryUnit.m_CpuInstances[leafNode.m_ObjectStartIndex];
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                return ::IsValidBVH<Triangle>(&m_blasTrees[treeInterval.m_StartIndex], m_geometryUnit.m_CpuIndices.size() / 3, trianglePtr, nullptr,
                    [this](const Triangle& triangle) -> AABB3D
                    {
                        //NOTE: this ONLY WORKS IF WE APPLIED OBEJCT LEAF NODE INDEX OFFSET TO BLAS TREES
                        //SO THEY INDEX INTO GLOBAL INDEX ARRAY AND NOT JUST LOCAL MESH ARRAY
                        return CalculateTriangleAABB(triangle, &m_geometryUnit.m_CpuVertices[0]);
                    }, nullptr, true);
            }, true);
    }

    void Renderer::SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix)
    {
        m_viewerUniformBuffer.TryWriteField("worldPos", sizeof(Vec3), worldPos.GetMemPointer());
        //Mat4 viewTransposed = viewMatrix.Transpose();
        if (!m_viewerUniformBuffer.TryWriteField("viewMatrix", sizeof(Mat4),
            viewMatrix.GetMemPointer()))
        {
            LogError(std::format("Attempted to write view matrix to viewer uniform buffer but failed"));
            return;
        }

        //for (size_t i = 0; i < 16; i++) std::cout << std::format("[{}]", projMatrix.GetMemPointer()[i]);
        //std::cout << "" << std::endl;
        //Mat4 projMatrixRight = projMatrix.Transpose();
        if (!m_viewerUniformBuffer.TryWriteField("projectionMatrix", sizeof(Mat4),
            projMatrix.GetMemPointer()))
        {
            LogError(std::format("Attempted to write projection matrix to viewer uniform buffer but failed"));
            return;
        }
        
        //std::array<float, 16> floats = {};
        //m_viewerUniformBuffer.TryReadField("projectionMatrix", &floats);
        //for (const auto& val : floats) std::cout<<(std::format("[{}]", val));
        //LogError(std::format("Wrote matrix: {} to proj actual values in storage order", projMatrix.ToString()));
        //m_viewerUniformBuffer.Get
        
    }
    void Renderer::SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix,
        const Vec3& forwardDir, const Vec3& rightDir, const Vec3& upDir, const float yFov)
    {
        SetViewerData(worldPos, viewMatrix, projMatrix);
        if (!m_viewerUniformBuffer.TryWriteField("forwardDir", sizeof(Vec3), forwardDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write forwardDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteField("rightDir", sizeof(Vec3), rightDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write rightDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteField("upDir", sizeof(Vec3), upDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write upDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteField("yFov", sizeof(float), &yFov))
        {
            LogError(std::format("Attempted to write yFov to viewer uniform buffer but failed"));
            return;
        }
    }
    void Renderer::UpdateUniformBuffers()
    {
        if (m_uniformData.m_LightBufferNeedsUpdate)
        {
            m_lightUniformBuffer.WriteData(0, sizeof(LightBlockData), &m_uniformData.m_LightBlock);
            m_uniformData.m_LightBufferNeedsUpdate = false;
        }
        //TODO: right now camera data is always written FIX THIS
        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        Vec3 worldFoward, worldUp, worldRight;
        camera.GetTransform().CalculateWorldDirections(&worldFoward, &worldUp, &worldRight);
        SetViewerData(camera.GetTransform().GetWorldPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix,
            worldFoward, worldRight, worldUp, camera.GetSettings().m_FieldOfViewYRadians);

        if (Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::ViewMatrix))
        {
            m_unmovingFrames = 0;
        }
    }

    void Renderer::DrawGeometryBatch(RenderBatch& batch)
    {
        Backend::DrawUploadedIndexBufferInstanced(0,
            batch.m_VertexStartIndex * m_geometryUnit.m_IndexBufferHandle.GetElementSize(),
            batch.m_InstanceCount, batch.m_InstanceStartIndex, batch.m_InstanceCount);
    }

    void Renderer::ExecuteSkyboxPass(std::uint8_t* outDrawnAttachmentsMask)
    {
        UpdatePassRenderState(RenderPassType::Skybox);
        if (m_boundFrameBuffer != nullptr)
        {
            m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &m_hdrColorOutput);
        }
        //NOTE: we only need to clear the background color and not depth since we ignore depth
        Backend::ClearBackground(0b1);
        if (outDrawnAttachmentsMask != nullptr)
            *outDrawnAttachmentsMask |= 0b1;

        Shader& skyboxShader = GetCoreShader(CoreShader::Skybox);
        BindShader(skyboxShader);

        SlotIndex slot = m_textureController.TryBindToFreeSlot<Texture>(*m_skybox);
        skyboxShader.TrySetUniform(UniformDataType::Sampler2D, SKYBOX_UNIFORM_NAME, &slot);

        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        Quat cameraRotation = camera.GetTransform().GetWorldRotation();
        const Vec3 forwardDir = cameraRotation.ApplyRotationToDir(ENGINE_FORWARD_DIR);
        const Vec3 rightDir = CrossProduct(ENGINE_UP_DIR, forwardDir).GetNormalized();
        Mat4 noTranslationViewMatrix = Utils::CalculateModelMatrix(nullptr, Vec3::Zero(), Vec3::One(), 
            Utils::CalculateRotationMatrix(forwardDir, ENGINE_UP_DIR, rightDir));
        //SetViewerData(camera.GetTransform().GetWorldPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix);
        SetViewerData(camera.GetTransform().GetWorldPos(), noTranslationViewMatrix, cameraData.m_PlatformProjectionMatrix);

        Backend::SetDepthTesting(false);
        Backend::SetDepthWriting(false);

        Backend::DrawVertices(36);

        Backend::SetDepthTesting(true);
        Backend::SetDepthWriting(true);
        m_textureController.TryRemoveFromSlot(slot);
        UnbindActiveShader();
    }

    void Renderer::ExecuteShadowPass()
    {
        UpdatePassRenderState(RenderPassType::Shadow);

        Shader& shadowShader = GetCoreShader(CoreShader::Shadow);
        //shadowShader->BindUniformBlockIfNeeded(m_viewerUniformBuffer.GetName(), m_viewerUniformBuffer.GetBindIndex());
        BindShader(shadowShader);
        
        std::array<Mat4, 6> lightViewMatrices = {};
        Mat4 lightProjMatrix = {};

        for (size_t i = 0; i < m_uniformData.m_LightBlock.m_PointLightsCount; i++)
        {
            auto& light = m_uniformData.m_LightBlock.m_PointLights[i];
            auto& otherLightData = m_uniformData.m_ExtraPointLightData[i];

            CalculateCubeMapMatrices(light.m_Pos, SHADOW_NEAR_DISTANCE, light.m_Radius, lightViewMatrices, lightProjMatrix);
            Backend::SetViewport(m_shadowMaps[i].GetData().m_texelSize.m_X, m_shadowMaps[i].GetData().m_texelSize.m_Y);

            //For every single face on cube, we redraw scene from light perspective
            for (size_t j = 0; j < 6; j++)
            {
                m_frameBuffer.SetOutputTextureCube(FrameBufferAttachmentType::Depth, &m_shadowMaps[i], static_cast<TextureCubeFace>(j));
                Backend::ClearBufferBit(BufferBitType::Depth);

                SetViewerData(light.m_Pos, lightViewMatrices[j], lightProjMatrix);

                for (size_t k = 0; k < m_geometryUnit.m_Batches.size(); k++)
                {
                    auto& batch = m_geometryUnit.m_Batches[k]; 
                    if (batch.m_InstanceCount == 0)
                        continue;

                    DrawGeometryBatch(batch);
                }
            }
        }
   
        UnbindActiveShader();
        //This forces the viewport to be set back to rendering for the window
        m_engineState->m_GraphicsContext.m_Window->ForceSizeUpdate();
    }
    void Renderer::ExecuteLightingAndGeometryPass(const SlotIndex* shadowCubeMapSlots, const std::uint8_t previousDrawnColorAttachmentsMask)
    {
        UpdatePassRenderState(RenderPassType::Geometry);
        if (m_boundFrameBuffer != nullptr)
        {
            m_boundFrameBuffer->SetOutputRenderBuffer(FrameBufferAttachmentType::Depth, &m_hdrDepthRenderBuffer);
            m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &m_hdrColorOutput);
            m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color1, &m_brightnessOutput);
        }
        //If we did not draw to color attachments before this, we clear the two colors attachments we will need, color0 and color1
        //to ensure we do not have previous frame color data here
        const std::uint8_t clearBitmask = 0b11;
        if (previousDrawnColorAttachmentsMask == 0) Backend::ClearBackground(clearBitmask);
        //If the previous draw mask is not 0, we want to ignore any bits it drew previously but we dont want to clear now
        //(so 0 bit in clear makes any bit 0 even if previousMask is 0 or 1 AND any 1 bit in clear stays 1 unless previous is also 1
        //so we only clear necessary slots that we need right now, but only if they were not previouslt drawn to)
        else Backend::ClearBackground(~(~clearBitmask | previousDrawnColorAttachmentsMask));
        UpdateUniformBuffers();

        const Texture* lastBatchTexture = nullptr;
        const auto removeLastBatchTexture = [this, &lastBatchTexture]() -> void
            {
                m_textureController.TryRemoveFromSlot(*lastBatchTexture);
                lastBatchTexture = nullptr;
            };
        
        Shader* lastBatchShader = nullptr;
        const auto unbindLastBatchShader = [this, &lastBatchShader]() -> void
            {
                UnbindActiveShader();
                lastBatchShader = nullptr;
            };


        for (int i = 0; i < m_geometryUnit.m_Batches.size(); i++)
        {
#ifdef GRAPHICS_VERBOSE_LOG
            LogWarning(std::format("Flushing batch:{}/{}", i + 1, m_batches.size()));
#endif
            auto& batch = m_geometryUnit.m_Batches[i];

            //If we have no instance data it means it might be a leftover batch from previous frame
            //that was not cleared
            if (batch.m_InstanceCount == 0)
                continue;

            if (batch.m_Shader == nullptr)
            {
                LogError(std::format("Tried to flush current batch in renderer, but batch shader was null"));
                return;
            }

            if (lastBatchShader != nullptr && batch.m_Shader == nullptr) unbindLastBatchShader();
            else if (lastBatchShader == nullptr || lastBatchShader != batch.m_Shader)
            {
                BindShader(*batch.m_Shader);
            }
            lastBatchShader = batch.m_Shader;

            //TODO: right now the current batch gets it texture slot by removing previous slot
            //but what if we use that texture in a future batch it would be wasteful -> so
            //we have to make sure that no future queued batches have last batch texture before removing
            if (lastBatchTexture != nullptr && lastBatchTexture != batch.m_Texture) removeLastBatchTexture();
            if (batch.m_Texture != nullptr && lastBatchTexture != batch.m_Texture)
            {
                SlotIndex slot = m_textureController.TryBindToFreeSlot<Texture>(*batch.m_Texture);

                if (!batch.m_Shader->TrySetUniform(UniformDataType::Sampler2D, TEXTURE_UNIFORM_NAME, &slot))
                    return;
            }
            if (DO_SHADOWS && batch.m_Shader == &GetCoreShader(CoreShader::ForwardRender))
            {
                batch.m_Shader->TrySetUniformArray(UniformDataType::CubeSampler, SHADOW_MAP_UNIFORM_NAME,
                    shadowCubeMapSlots, m_uniformData.m_LightBlock.m_PointLightsCount);
            }
            
            lastBatchTexture = batch.m_Texture;
            //LogWarning(std::format("LIGHT PASS Texture controler before draw: {}", m_textureController.ToString()));

            //DrawBatch(batch);
            DrawGeometryBatch(batch);
        }

        if (lastBatchShader != nullptr) unbindLastBatchShader();
        if (lastBatchTexture != nullptr) removeLastBatchTexture();

        //LogWarning(std::format("LIGHT PASS Texture controler after pass: {}", m_textureController.ToString()));
        //LogError(std::format("HDR color texture: {}", Utils::ToStringMemory(writePtr, byteSize)));
    } 

    void Renderer::ExecuteForwardRendering()
    {
        const bool hasSkybox = m_skybox != nullptr;
        std::uint8_t previousDrawnAttachmentsMask = 0;
        if (hasSkybox) ExecuteSkyboxPass(&previousDrawnAttachmentsMask);

        //NOTE: we do this to ensure that we only add any data as long as all 3 buffers have enough space
        const auto& freeVertexSeg = m_geometryUnit.m_VertexBufferHandle.TryGetFreeSegment(m_geometryUnit.GetVertexCount());
        const auto& freeIndexSeg = m_geometryUnit.m_IndexBufferHandle.TryGetFreeSegment(m_geometryUnit.GetIndexCount());
        const auto& freeInstanceSeg = m_geometryUnit.m_InstanceBufferHandle.TryGetFreeSegment(m_geometryUnit.GetInstanceCount());
        if (freeVertexSeg == std::nullopt || freeIndexSeg == std::nullopt || freeInstanceSeg == std::nullopt)
        {
            m_isRenderStalled = true;
            const std::string message = std::format("Stalling vertex:{} index:{} instance:{}",
                freeVertexSeg == std::nullopt, freeIndexSeg == std::nullopt, freeIndexSeg == std::nullopt);
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }

        FencedBufferSegment& vertexFenceSeg = m_geometryUnit.m_VertexBufferHandle.WriteDataFenced(m_geometryUnit.GetVertexMemPointer(), freeVertexSeg.value());
        FencedBufferSegment& indexFenceSeg = m_geometryUnit.m_IndexBufferHandle.WriteDataFenced(m_geometryUnit.GetIndexMemPointer(), freeIndexSeg.value());
        FencedBufferSegment& instanceFenceSeg = m_geometryUnit.m_InstanceBufferHandle.WriteDataFenced(m_geometryUnit.GetInstanceMemPointer(), freeInstanceSeg.value());

        std::vector<SlotIndex> shadowCubeMapSlots = {};
        if (DO_SHADOWS)
        {
            ExecuteShadowPass();

            vertexFenceSeg.m_Fence.Insert();
            indexFenceSeg.m_Fence.Insert();
            instanceFenceSeg.m_Fence.Insert();

            const size_t totalPointLights = m_uniformData.m_LightBlock.m_PointLightsCount;
            shadowCubeMapSlots = m_textureController.TryBindToFreeSlots<TextureCube>(m_shadowMaps, totalPointLights);

            if (shadowCubeMapSlots.empty() || shadowCubeMapSlots.size() != totalPointLights)
            {
                LogError(std::format("Attempted to add shadow map textures to available slots but failed."
                    "Reserved slots:{} expected size:{}", shadowCubeMapSlots.size(), totalPointLights));
                return;
            }
        }

        ExecuteLightingAndGeometryPass(DO_SHADOWS ? &shadowCubeMapSlots[0] : nullptr, previousDrawnAttachmentsMask);
        if (DO_SHADOWS) m_textureController.RemoveFromSlots(shadowCubeMapSlots);
        else
        {
            vertexFenceSeg.m_Fence.Insert();
            indexFenceSeg.m_Fence.Insert();
            instanceFenceSeg.m_Fence.Insert();
        }
    }

    void Renderer::ExecutePostProcessPass()
    {
        UpdatePassRenderState(RenderPassType::PostProcess);

        if (DO_BLOOM)
        {
            ApplyBlurInPlace(m_brightnessOutput, m_ioTexture, 2);
        }

        /*LogError(std::format("Bound fraembuffer: {} size: {}", Backend::GetRenderObjectId(RenderObjectQueryType::BoundFrameBuffer),
            Backend::GetViewportSize().ToString()));*/

        Shader& ppShader = GetCoreShader(CoreShader::PostProcess);
        BindShader(ppShader);

        SlotIndex bloomSlotIndex = INVALID_SLOT_INDEX;
        if (DO_BLOOM)
        {
            bloomSlotIndex = m_textureController.TryBindToFreeSlot<Texture>(m_brightnessOutput);
            ppShader.TrySetUniform(UniformDataType::Sampler2D, BRIGHTNESS_TEXTURE_UNIFORM_NAME, &bloomSlotIndex);
        }

        //TODO: this is inneficient to bind the output texture to slot when we had to bind it during blur in place
        //so we should either FORCE bind before blur in place, or return texture slot after blur in place
        const SlotIndex hdrOutputIndex = m_textureController.TryBindToFreeSlot<Texture>(m_hdrColorOutput);
        ppShader.TrySetUniform(UniformDataType::Sampler2D, HDR_TEXTURE_UNIFORM_NAME, &hdrOutputIndex);

        //NOTE: we disable depth testing since we only draw one full screen triangle ( + its faster) and
        //since hdr draws geometry to custom frame buffer, the depth in DEFAULT fbo would be 0, thus all 
        //fragments would fail test -> result in full screen black even if backbuffer color is right
        Backend::SetDepthTesting(false);
        Backend::SetSrgbConversionStatus(true);
        
        //We just draw 3 vertices -> note we do not need vertex buffer since we use vertices defined in vertex shader
        Backend::DrawVertices(3);

        //NOTE: always unbind shader before switching shader settings like depth or srgb conversion status
        UnbindActiveShader();
        Backend::SetDepthTesting(true);
        Backend::SetSrgbConversionStatus(false);

        m_textureController.TryRemoveFromSlot(hdrOutputIndex);
        if (bloomSlotIndex != INVALID_SLOT_INDEX) m_textureController.TryRemoveFromSlot(bloomSlotIndex);
    }

    void Renderer::ApplyBlurInPlace(Texture& inputTexture, Texture& tempTexture, const float blurStrength)
    {
        //Gaussian blur function: G(x)=e ^ -(x^2 / 2o^2)
        //where x = distance (in pixels) from starting pixel
        //o (sigma) controls the spread (larger = blurrier)
        //You normalize all weights so they sum to 1
        constexpr int WEIGHT_SIZE = 5;
        constexpr int RADIUS = WEIGHT_SIZE - 1;
        float weights[WEIGHT_SIZE] = {};

        float sum = 0.0f;

        for (int i = 0; i <= RADIUS; i++)
        {
            weights[i] = std::exp(-float(i * i) / (2.0f * blurStrength * blurStrength));
            sum += (i == 0) ? weights[i] : 2.0f * weights[i];
        }
        for (int i = 0; i <= RADIUS; i++)
            weights[i] /= sum;

        if (inputTexture.GetInfo().m_texelSize != tempTexture.GetInfo().m_texelSize)
        {
            LogError(std::format("Attempted to apply blur for texture:{} to output:{} "
                "but they have different texel sizes", inputTexture.ToString(), tempTexture.ToString()));
            return;
        }
        if (tempTexture.GetInfo().m_internalStorage != TexelStorageType::RGBA16F)
        {
            LogError(std::format("Attempted to apply blur for texture:{} to output:{} "
                "but output texture does not have required rgba16 storage format", 
                inputTexture.ToString(), tempTexture.ToString()));
            return;
        }

        Shader& blurShader = GetCoreShader(CoreShader::GaussianBlur);
        BindShader(blurShader);

        blurShader.TrySetUniformArray(UniformDataType::Float, BLUR_WEIGHTS_UNIFORM_NAME, &weights[0], 5);

        //-------------------------------------------------
        // HORIZONTAL PASS
        //-------------------------------------------------
        bool isHorizontalPass = true;
        blurShader.TrySetUniform(UniformDataType::Bool, HORIZONTAL_FLAG_UNIFORM_NAME, &isHorizontalPass);

        SlotIndex inputTextureSlot = m_textureController.TryBindToFreeSlot<Texture>(inputTexture);
        blurShader.TrySetUniform(UniformDataType::Sampler2D, INPUT_TEXTURE_UNIFORM_NAME, &inputTextureSlot);
        SlotIndex outputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(tempTexture, AccessPermissions::Write);
        blurShader.TrySetUniform(UniformDataType::Image2D, OUTPUT_TEXTURE_UNIFORM_NAME, &outputTextureSlot);

        //Dispatch one thread per pixel where each group size is defined in shader 
        //NOTE: since we do division, we may lose some texels, so we increase it to ensure we account for remainderc
        const Vec3Int computeGroups = Vec3Int(inputTexture.GetInfo().m_texelSize, 1);
        blurShader.DispatchComputeShaderGroups(computeGroups);
        //We must invoke memory sync to ensure image operation applied to OUTPUT texture go through before
        //using it for sampling as INPUT texture for vertical pass
        Backend::InvokeImageMemorySync(ImageOperationBarrierType::TextureFetch);

        //-------------------------------------------------
        // VERTICAL PASS
        //-------------------------------------------------
        isHorizontalPass = false;
        blurShader.TrySetUniform(UniformDataType::Bool, HORIZONTAL_FLAG_UNIFORM_NAME, &isHorizontalPass);

        //NOTE: for vertical pass we ONLY swap the input output textures so horizontal pass output is now input
        //in order to save space (we CANT use one texture since we then get incorrect results)
        // AND we DONT need to update uniforms since the slot bindings stay the same, only textures in slots swap
        m_textureController.RebindAtSlot<Texture>(inputTextureSlot, tempTexture);
        m_imageController.RebindAtSlot<Texture>(outputTextureSlot, inputTexture, AccessPermissions::Write);

        blurShader.DispatchComputeShaderGroups(computeGroups);

        UnbindActiveShader();
        if (!m_textureController.TryRemoveFromSlot(inputTextureSlot))
        {
            LogError(std::format("Attempted to remove texture from binded slot: {} but failed", inputTextureSlot));
        }
        if (!m_imageController.TryRemoveFromSlot(outputTextureSlot))
        {
            LogError(std::format("Attempted to remove texture from binded slot: {} but failed", inputTextureSlot));
        }

        //The next operation to use the input texture (remember INPUT is now the OUTPUT texture after vertical pass)
        //will most liekly be the post process draw call
        Backend::InvokeImageMemorySync(ImageOperationBarrierType::FrameBuffer);
    }

    void Renderer::ExecuteRayTracing()
    {
        UpdatePassRenderState(RenderPassType::RayTrace);
        Shader& rayTraceShader = GetCoreShader(CoreShader::RayTrace);
        rayTraceShader.BindActive();
        //LogWarning("BOUND RAY TRACE");

        //If we dont do static goemetry we write every frame, otherwise
        //we only write during the first geometry init
        if (!DO_STATIC_GEOMETRY || (DO_STATIC_GEOMETRY && m_framesSinceStart == 0)) 
            WriteGeometryVertexDataToSSBOs();

        //TODO: get propert emissive material count
        const std::uint32_t emissiveMaterialCount = m_emissiveInstanceIndices.size();
        rayTraceShader.TrySetUniform(UniformDataType::Uint, EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME, &emissiveMaterialCount);

        ENGINE_ASSERT(m_geometryUnit.m_CpuInstances.size() != 0, "Attempted to execute ray tracing but there are no instances");

        const SlotIndex inputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(m_hdrColorOutput, AccessPermissions::ReadWrite);
        const SlotIndex outputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(m_ioTexture, AccessPermissions::Write);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, INPUT_TEXTURE_UNIFORM_NAME, &inputTextureSlot);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, OUTPUT_TEXTURE_UNIFORM_NAME, &outputTextureSlot);

        rayTraceShader.TrySetUniform(UniformDataType::Uint, UNMOVING_FRAME_NUMBER_UNIFORM_NAME, &m_unmovingFrames);
        m_lightUniformBuffer.WriteData(0, sizeof(LightBlockData), &m_uniformData.m_LightBlock);

        SlotIndex bloomSlot = INVALID_SLOT_INDEX;
        if (DO_BLOOM)
        {
            bloomSlot = m_imageController.TryBindToFreeSlot<Texture>(m_brightnessOutput, AccessPermissions::Write);
            rayTraceShader.TrySetUniform(UniformDataType::Image2D, BRIGHTNESS_IMAGE_UNIFORM_NAME, &bloomSlot);
        }

        const bool doSkybox = m_skybox != nullptr;
        rayTraceShader.TrySetUniform(UniformDataType::Bool, "uHasSkybox", &doSkybox);
        SlotIndex skyboxSlot = INVALID_SLOT_INDEX;
        if (doSkybox)
        {
            skyboxSlot = m_textureController.TryBindToFreeSlot<Texture>(*m_skybox);
            rayTraceShader.TrySetUniform(UniformDataType::Sampler2D, SKYBOX_UNIFORM_NAME, &skyboxSlot);
        }
        
        int* textureSampleSlots = (int*)alloca(sizeof(int) * m_bindQueuedTextures.size());
        if (!m_bindQueuedTextures.empty())
        {            
            for (std::uint8_t i = 0; i < m_bindQueuedTextures.size(); i++)
            {
                SlotIndex slotIndex = m_textureController.TryBindToFreeSlot<Texture>(*m_bindQueuedTextures[i]);
                if (slotIndex == INVALID_SLOT_INDEX)
                {
                    LogError("Attempted to bind a texture for raytracing but failed");
                    return;
                }
                textureSampleSlots[i] = slotIndex;
            }

            rayTraceShader.TrySetUniformArray(UniformDataType::Sampler2D,
                TEXTURES_UNIFORM_NAME, textureSampleSlots, m_bindQueuedTextures.size());
        }
        /*
        static int times = 0;
        times++;
        if (times == 1)
        {
            LogWarning(std::format("RAYTRACE RENDER\nViewer: \nVertices:{}\nIndices:{}\nInstances:{}\nInstanceMeshes:{}\nLightIndices:{}\nMaterials:{}\n"
                "CAMERA:\nPos:{}\nView:{}\nProj:{}\nForward:{} Up:{} Right:{}\nYFov:{}\n",
                Utils::ToStringIterable<std::vector<VertexType>, VertexType>(m_geometryUnit.m_CpuVertices),
                Utils::ToStringIterable<std::vector<IndexType>, IndexType>(m_vertexIndices),
                Utils::ToStringIterable<std::vector<Instance>, Instance>(m_instances),
                Utils::ToStringIterable<std::vector<InstanceMesh>, InstanceMesh>(m_instanceMeshes),
                Utils::ToStringIterable<std::vector<std::uint32_t>, std::uint32_t>(m_emissiveInstanceIndices),
                Utils::ToStringIterable<std::vector<MaterialData>, MaterialData>(m_materialData),
                camera.GetTransform().GetGlobalPos().ToString(), cameraData.m_ViewMatrix.ToString(), 
                cameraData.m_PlatformProjectionMatrix.ToString(), worldFoward.ToString(), worldUp.ToString(), worldRight.ToString(),
                camera.GetSettings().m_FieldOfViewYRadians
            ));
        }
        */

        UpdateUniformBuffers();
        
        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
        rayTraceShader.DispatchComputeShaderGroups(Vec3Int(windowSize, 1));

        //We must invoke memory sync to ensure image operation applied to OUTPUT texture go through before
        //using it for applying it to the hdr color output
        Backend::InvokeImageMemorySync(ImageOperationBarrierType::TextureFetch);
        rayTraceShader.UnbindActive();

        m_imageController.TryRemoveFromSlot(inputTextureSlot);
        m_imageController.TryRemoveFromSlot(outputTextureSlot);
        if (bloomSlot != INVALID_SLOT_INDEX) m_imageController.TryRemoveFromSlot(bloomSlot);
        if (skyboxSlot != INVALID_SLOT_INDEX) m_textureController.TryRemoveFromSlot(skyboxSlot);

        for (std::uint8_t i = 0; i < m_bindQueuedTextures.size(); i++)
        {
            m_textureController.TryRemoveFromSlot(textureSampleSlots[i]);
        }
    }

    void Renderer::FlushBatches()
    {
        if (RENDER_FRAMES_COUNT != NO_RENDER_FRAME_COUNT_LIMIT &&
            m_framesSinceStart >= RENDER_FRAMES_COUNT)
        {
            LogError(std::format("Reached render frame count of: {}", m_framesSinceStart));
            return;
        }
        
        if (m_framesSinceStart == 0)
        {
            ConstructTLASTree();
            m_materialStorageBuffer.WriteData(0, m_materialData.size() * sizeof(MaterialData), &m_materialData[0]);
        }
        //TestBVHIntersectionSphere(*this);

        std::function<bool(Vec3, Vec3)> testFunc =
            [this](Vec3 rayWorldOrigin, Vec3 rayDir) -> bool
            {
                return IntersectsBVH(rayWorldOrigin, rayDir, nullptr);
            };
            
        /*
        const auto& transform = m_engineState->m_CameraController->GetActiveCamera().GetTransform();
        LogSimple("Intersects {} -> {}: {}", transform.GetWorldPos().ToString(), transform.CalculateWorldForward().ToString(),
            IntersectsBVH(transform.GetWorldPos(), transform.CalculateWorldForward(), nullptr));
        */
        //LogWarning(std::format("Frame number: {}", m_framesSinceStart));

        if (DO_RAYTRACING)
        {
            ExecuteRayTracing();
            //LogError(std::format("FULL TREE: {} \nTLAS NODES:{}", ToStringBVH(), Utils::ToStringIterable(m_tlasTree.GetNodes())));
        }
        else ExecuteForwardRendering();

        //TODO: you should be able to do pp without hdr too
        if (DO_POST_PROCESS) ExecutePostProcessPass();
    }

    void Renderer::RenderBuffer()
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        //BLOOM_THRESHOLD = 2 * std::abs(std::sin(m_engineState->m_TimeKeeper->GetTimeSinceInit(TimeUnit::Seconds)));
        //LogWarning(std::format("BLoom is: {} now: {}", BLOOM_THRESHOLD, m_engineState->m_TimeKeeper->GetTimeSinceInit(TimeUnit::Milliseconds)));
        //LogWarning("RENDER");
        if (DO_VISUALIZE_BVH_BOUNDS) AddBVHTreeBoundsWireframe();
        if (!m_geometryUnit.m_Batches.empty())
        {
            FlushBatches();
        }
        RenderEndActions();
    }

    void Renderer::RenderEndActions()
    {
        m_uniformData.m_ViewBufferNeedsUpdate = false;

        m_framesSinceStart++;
        m_unmovingFrames++;
        
        if (!DO_STATIC_GEOMETRY)
        {
            m_uniformData.m_LightBlock.m_PointLightsCount = 0;
            m_frameGeometryMetrics = {};

            m_geometryUnit.m_CpuVertices.clear();
            m_geometryUnit.m_CpuIndices.clear();
            m_geometryUnit.m_CpuInstances.clear();
            m_instanceMeshes.clear();
            m_emissiveInstanceIndices.clear();
            m_geometryUnit.m_Batches.clear();

            //TODO: is this the best option for perofmrance amd should all be force cleared?
            //ideally we want to remove from middle, but that forces shifts in memory which may greatloy reduce performance
            //Tradeoff: performance cost for erasing some > performance cost of clearing all/having to reallocate frequent batches?
            m_geometryUnit.m_Batches.clear();
            ResetRuntimeMaterialId();
        }
        
        UpdatePassRenderState(RenderPassType::None);
    }

    std::string Renderer::ToStringBVH() const
    {
        return "TO STRING BVH: \n" + m_tlasTree.ToString(BVHToStringType::NodeBounds, nullptr,
            //TLAS leaf node to string function 
            [this](const BVHFlatNode& tlasLeafNode) -> std::string
            {
                const Instance& instance = m_geometryUnit.m_CpuInstances[tlasLeafNode.m_ObjectStartIndex];
                const InstanceMesh& mesh = m_instanceMeshes[instance.m_MeshIndex];
                const ArrayInterval interval = mesh.m_BLASTreesInterval;
                const Vec3 aabbSize = tlasLeafNode.GetAABB().GetSize();
                if (Utils::ApproximateEqualsF(aabbSize.m_X, 0) || Utils::ApproximateEqualsF(aabbSize.m_Y, 0) || Utils::ApproximateEqualsF(aabbSize.m_Z, 0))
                    LogWarning(std::format("[BVH]: Found INVALID 0-value TLAS tree node: {}", tlasLeafNode.ToString()));

                //Here we add a red prefix if the split between the leaf node of the tlas tree and the transformed 
                //world root node bounds of the blas tree is wrong (the tlas leaf node bounds > blas root node bounds)
                std::string invalidBoundsPrefix = "";
                std::string invalidBoundsSuffix = "";
                const BVHFlatNode& blasRootNode = m_blasTrees[interval.m_StartIndex];
                AABB3D rootNodeWorldBounds = Utils::ApplyMatrixToAABB(blasRootNode.GetAABB(), instance.m_ModelMatrix);
                AABB3D parentBounds = tlasLeafNode.GetAABB();
                if (parentBounds.GetSize().AnyAxisLessThan(rootNodeWorldBounds.GetSize()) || 
                    parentBounds.m_MinPos.AnyAxisGreaterThan(rootNodeWorldBounds.m_MinPos) ||
                    parentBounds.m_MaxPos.AnyAxisLessThan(rootNodeWorldBounds.m_MaxPos))
                {
                    invalidBoundsPrefix = ANSI_COLOR_RED;
                    invalidBoundsSuffix = ANSI_COLOR_CLEAR;
                }

                //LogWarning(std::format("Interval is: {} mesh index: {} instance index:{}", interval.m_Size, m_instances[node.m_ObjectStartIndex].m_MeshIndex));
                const Triangle* trianglePtr = reinterpret_cast<const Triangle*>(m_geometryUnit.m_CpuIndices[0]);
                return invalidBoundsPrefix + ToStringBVHNodes<Triangle>(&m_blasTrees[0], interval.m_StartIndex,
                    interval.m_Size, trianglePtr, nullptr, BVHToStringType::NodeBounds,
                    [instance, &tlasLeafNode](const BVHFlatNode& blasNode, const BVHFlatNode* parentNode) -> std::string
                    {
                        const Vec3 aabbSize = blasNode.GetAABB().GetSize();
                        if (Utils::ApproximateEqualsF(aabbSize.m_X, 0) || Utils::ApproximateEqualsF(aabbSize.m_Y, 0)
                            || Utils::ApproximateEqualsF(aabbSize.m_Z, 0))
                        {
                            LogWarning(std::format("[BVH]: Found INVALID 0-value bounds for BLAS tree node: {}", blasNode.ToString()));
                        }

                        return std::format("[BLASNode Bounds:{}]",
                            Utils::ApplyMatrixToAABB(blasNode.GetAABB(), instance.m_ModelMatrix).ToString());
                    },
                    //BLAS Leaf node to string function -> get vertices
                    [this, &instance, &mesh, &blasRootNode](const BVHFlatNode& blasLeafNode) -> std::string
                    {
                        //The object indices for blas leaves are TRIANGLE INDICES
                        const size_t indexStartIndex = blasLeafNode.m_ObjectStartIndex * 3;
                        const size_t indexCount = blasLeafNode.m_ObjectCount * 3;
                        if (indexStartIndex < mesh.m_IndexOffset ||
                            indexStartIndex + indexCount > mesh.m_IndexOffset + mesh.m_NumIndices)
                        {
                            LogWarning(std::format("[BVH]: Found BLAS tree leaf node (index:{}) in invalid mesh range. Leaf index start:{} count:{} "
                                "Mesh index start:{} count:{} total indices:{}", size_t(&blasLeafNode - &blasRootNode),
                                indexStartIndex, indexCount, mesh.m_IndexOffset, mesh.m_NumIndices, m_geometryUnit.m_CpuIndices.size()));
                        }

                        std::string verticesStr = "";
                        for (size_t i = 0; i < indexCount; i += 3)
                        {
                            const Vec3& v0 = m_geometryUnit.m_CpuVertices[m_geometryUnit.m_CpuIndices[indexStartIndex + i]].m_LocalPos;
                            const Vec3& v1 = m_geometryUnit.m_CpuVertices[m_geometryUnit.m_CpuIndices[indexStartIndex + i + 1]].m_LocalPos;
                            const Vec3& v2 = m_geometryUnit.m_CpuVertices[m_geometryUnit.m_CpuIndices[indexStartIndex + i + 2]].m_LocalPos;
                               
                            std::string triangleStr = std::format("[Triangle V0:{} V1:{} V2:{}]",
                                (instance.m_ModelMatrix * Vec4(v0, 1)).GetXYZ().ToString(),
                                (instance.m_ModelMatrix * Vec4(v1, 1)).GetXYZ().ToString(),
                                (instance.m_ModelMatrix * Vec4(v2, 1)).GetXYZ().ToString());

                            if (!Utils::IsWithinBounds(blasLeafNode.GetAABB(), v0) || !Utils::IsWithinBounds(blasLeafNode.GetAABB(), v1) ||
                                !Utils::IsWithinBounds(blasLeafNode.GetAABB(), v2))
                            {
                                //LogError(std::format("[BVH]"));
                                verticesStr += ANSI_COLOR_RED + triangleStr + ANSI_COLOR_CLEAR;
                            }
                            if (Utils::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v0) || Utils::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v1) ||
                                Utils::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v2))
                            {
                                //LogError(std::format("[BVH]"));
                                static int count = 0;
                                count++;
                                LogWarning(std::format("Found one completely outside boudns:{} TOTLA:{}/{}", 
                                    size_t(&blasLeafNode - &blasRootNode), count, mesh.m_NumIndices / 3));
                            }
                            else
                            {
                                //LogWarning(std::format("FOUND GOOD at:{}", size_t(&blasLeafNode - &blasRootNode)));
                                verticesStr += triangleStr;
                            }
                        }
                        return verticesStr;
                    }, true) + invalidBoundsSuffix;
            }, true);
    }
    std::string Renderer::ToStringInstances() const
    {
        std::string result = "INSTANCES:";
        for (const auto& batch : m_geometryUnit.m_Batches)
        {
            result += std::format("\nNew BATCH: Vertices{} Indices:{}", batch.m_VertexCount, m_geometryUnit.m_CpuIndices.size());
            for (size_t i = 0; i < batch.m_InstanceCount; i++)
            {
                const Instance& instance = m_geometryUnit.m_CpuInstances[batch.m_InstanceStartIndex + i];
                const ArrayInterval interval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                const Triangle* trianglePtr = reinterpret_cast<const Triangle*>(&m_geometryUnit.m_CpuIndices[0]);
                std::string blasTreeString = ToStringBVHNodes<Triangle>(&m_blasTrees[0], interval.m_StartIndex,
                    interval.m_Size, trianglePtr, nullptr, BVHToStringType::NodeBounds);

                std::vector<Vec3> vertexPositions = {};
                const InstanceMesh& mesh = m_instanceMeshes[instance.m_MeshIndex];
                for (size_t j = 0; j < mesh.m_NumIndices; j++)
                {
                    vertexPositions.emplace_back(m_geometryUnit.m_CpuVertices[m_geometryUnit.m_CpuIndices[mesh.m_IndexOffset + j]].m_LocalPos);
                }

                result += std::format("\n[Instance]: Material(Idx:{}):{} Model:{} Vertices:{} \nMesh BLAS TREE(MeshIndex:{} IntervalStart:{} IntervalSize:{}):{}", 
                    instance.m_MaterialIndex, m_materialData[instance.m_MaterialIndex].ToString(), instance.m_ModelMatrix.ToString(),
                    Utils::ToStringIterable(vertexPositions), instance.m_MeshIndex, interval.m_StartIndex, interval.m_Size, blasTreeString);
            }
        }
        return result;
    }
    std::string Renderer::ToStringMetrics() const
    {
        std::string result = std::format("Metrics:\n Render calls:{}", 
            Utils::ToStringIterable(m_frameGeometryMetrics.m_RenderCallInvocations));
        //TODO: count how many sphere counts, etc
        return result;
    }
    std::string Renderer::ToStringAll() const
    {
        return std::format("DUMPING RENDERER DATA:\nCameraState:{}\nGEOMETRY DATA:\nVertex({}):{}\nIndex({}):{}\nInstances({}):{}\nBatches:{}", 
            m_engineState->m_CameraController->GetActiveCamera().ToString(),
            m_geometryUnit.m_CpuVertices.size(), Utils::ToStringIterable(m_geometryUnit.m_CpuVertices),
            m_geometryUnit.m_CpuIndices.size(), Utils::ToStringIterable(m_geometryUnit.m_CpuIndices),
            m_geometryUnit.m_CpuInstances.size(), Utils::ToStringIterable(m_geometryUnit.m_CpuInstances),
            m_geometryUnit.ToStringBatches());
    }
}