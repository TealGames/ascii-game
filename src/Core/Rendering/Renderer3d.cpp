//NOT USED
#include "pch.hpp"
#include <optional>
#include <queue>
#include "StaticGlobals.hpp"
#include "Core/Rendering/Renderer3d.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Utils/Debug.hpp"
#include "Core/EngineState.hpp"
#include "Core/Camera/CameraController.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Math/PlatformMath.hpp"
#include "Core/Window/WindowManager.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Math/Math3d.hpp"
#include "Tests/UnitTests.hpp"

namespace Engine::Rendering
{
    /// <summary>
    /// If true, all verticies must be present at the start before first frame update
    /// </summary>
    constexpr bool DO_STATIC_GEOMETRY = true;
    constexpr bool USE_CACHED_SHAPE_ASSETS = true;

    constexpr bool DO_RAYTRACING = true;
    constexpr RaytraceMode RAYTRACE_MODE = RaytraceMode::GPU;
    constexpr const char* OUTPUT_RAYTRACE_TEXTURE_PARENT_PATH = CURRENT_SOURCE_DIR "output/raytracer/";
    constexpr int OUTPUT_RAYTRACE_FRAME = 0;
    constexpr std::uint32_t MAX_RAYTRACE_BOUNCES = 5;
    constexpr std::uint32_t RAYTRACER_SAMPLES_PER_PIXEL = 1;

    constexpr bool DO_VISUALIZE_BVH_BOUNDS = false;
    constexpr float BVH_BOUNDS_LINE_THICKNESS = 1;
    constexpr ColHDR4 BVH_BOUNDS_COLOR = COLOR_GREEN;
    constexpr ColHDR4 BVH_BOUNDS_LEAF_COLOR = COLOR_RED;

    constexpr bool DO_LIGHTING = true;
    constexpr bool DRAW_LIGHT_AREAS = false;
    constexpr bool USE_LIGHT_COLOR_FOR_RANGE = true;
    constexpr ColHDR4 LIGHT_AREA_COLOR = {255, 255, 255, 255};
    
    constexpr bool DO_SHADOWS = false;
    constexpr Vec2Int SHADOW_MAP_SIZE = {256, 256};
    constexpr Vec2Int SKYBOX_MAP_SIZE = {512, 512};
    constexpr float SHADOW_NEAR_DISTANCE = 0.001;
    constexpr float SHADOW_FAR_DISTANCE = 1000;

    constexpr bool DO_POST_PROCESS = true;
    constexpr bool DRAW_UI = false;
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
       
    constexpr RenderLimitBehavior RENDER_LIMIT_BEHAVIOR = RenderLimitBehavior::Warn;
    constexpr size_t INSTANCE_MAX_COUNT = 16;
    constexpr size_t INDEX_MAX_COUNT = 200'000;
    constexpr size_t VERTEX_MAX_COUNT = 100'000;
    constexpr size_t UI_INSTANCE_MAX_COUNT = 50;
    constexpr size_t UI_INDEX_MAX_COUNT = 20'000;
    constexpr size_t UI_VERTEX_MAX_COUNT = 10'000;

    constexpr size_t MATERIAL_MAX_COUNT = 10;
    constexpr size_t TEXTURE_MAX_COUNT = 5;
    constexpr size_t BLAS_NODE_MAX_COUNT = 50'000;
    constexpr size_t TLAS_NODE_MAX_COUNT = 100;
    constexpr size_t CIRCLE_SIDE_COUNT = 12;

    static const char* CORE_SHADER_NAMES[CORE_SHADER_COUNT] = { 
        "default", "forward_render", "shadow", "texture", 
        "post_process", "gaussian_blur", "raytracer", "skybox", "ui"};

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
    constexpr const char* OUTPUT_TEXTURE_0_UNIFORM_NAME = "uTextureOutput0";
    constexpr const char* OUTPUT_TEXTURE_1_UNIFORM_NAME = "uTextureOutput1";
    constexpr const char* BLUR_WEIGHTS_UNIFORM_NAME = "uWeights";

    constexpr const char* RAY_TRACING_MAX_RAY_BOUNCES_UNIFORM_NAME = "uMaxBounces";
    constexpr const char* RAY_TRACING_SAMPLES_PER_PIXEL_UNIFORM_NAME = "uSamplesPerPixel";
    constexpr const char* UNMOVING_FRAME_NUMBER_UNIFORM_NAME = "uUnmovingFrameCount";
    constexpr const char* RAYTRACER_OUTPUT_TEXTURE_COUNT_UNIFORM_NAME = "uOutputTextureCount";
    constexpr const char* EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME = "uEmissiveCount";
    constexpr const char* INSTANCE_COUNT_UNIFORM_NAME = "uInstanceCount";

    bool RenderPassData::UsesDefaultFrameBuffer() const
    {
        return m_FrameBuffer != nullptr;
    }

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

    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants
    Renderer::Renderer(const Core::EngineState& engineState)
        : m_isInit(false), m_engineState(&engineState), m_uniformData(), m_skybox(),//m_staticRenderData(),
        m_graphicsManager(nullptr), m_frameGeometryMetrics(), m_runtimeMaterialId(),m_cachedMaterials(),
        m_textureController(Backend::CreateTextureController()),
        m_imageController(Backend::CreateImageController()),
        m_instanceMeshes(), m_emissiveInstanceIndices(), m_raytracer(),
        m_geometryVertexLayout(), m_uiVertexLayout(), m_bufferController(),
        m_geometryUnit(m_geometryVertexLayout), m_uiUnit(m_uiVertexLayout),
        m_unmovingFrames(0), m_isRenderStalled(false), m_framesSinceStart(0), m_cpuBufferTextures(),
        m_frameBuffer(), m_shadowMaps(), m_ColHDR4Output(), m_texRaytraceAccum0(), m_texRaytraceAccum1(), m_hdrDepthRenderBuffer(), m_coreShaders({}),
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
            //NOTE: we ALWAYS use our own framebuffer so we have more control
            if (data.m_PassType == RenderPassType::Shadow || data.m_PassType == RenderPassType::Skybox ||
                data.m_PassType == RenderPassType::Geometry)
                data.m_FrameBuffer = &m_frameBuffer;
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
        m_geometryUnit.Init(VERTEX_MAX_COUNT, INDEX_MAX_COUNT, INSTANCE_MAX_COUNT, RENDER_LIMIT_BEHAVIOR);

        const VertexLayoutBindIndex vertexBindIndex = m_bufferController.AddVertexBuffer(&m_geometryVertexLayout, 
            &m_geometryUnit.m_VertexBufferHandle, &m_geometryUnit.m_IndexBufferHandle);
        std::vector<VertexAttribute> vertexAttributes =
        {
            VertexAttribute(0, 3, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_LocalPos)),
            VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_UVPos)),
            VertexAttribute(2, 3, VertexAttributeBaseType::Float, false, offsetof(Vertex, m_Normal)),
        };
        m_geometryVertexLayout.AddAttributes(vertexBindIndex, vertexAttributes);

        const VertexLayoutBindIndex instancedBindIndex = m_bufferController.AddVertexBuffer(&m_geometryVertexLayout, 
            &m_geometryUnit.m_InstanceBufferHandle, nullptr);
        std::vector<VertexAttribute> instancedAttributes =
        {
            VertexAttribute(3, 1, VertexAttributeBaseType::UnsignedInteger, false, offsetof(Instance, m_MaterialIndex)),
            VertexAttribute(4, 1, VertexAttributeBaseType::UnsignedInteger, false, offsetof(Instance, m_MeshIndex)),
        };
        m_geometryVertexLayout.AddAttributes(instancedBindIndex, instancedAttributes);
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 5, false, sizeof(Vec4), offsetof(Instance, m_ModelMatrix));
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 9, false, sizeof(Vec4), offsetof(Instance, m_InverseModelMatrix));
        //NOTE: since the normal model matrix has 4 bytes of extra padding per column due to std::430 rules (which we only need for ssbo NOT
        //vertex buffer), the vertex layout needs full size INCLUDING padding
        m_geometryVertexLayout.AddMatrixAttribute(Vec2Int(3, 3), instancedBindIndex, 13, false, sizeof(Vec4), offsetof(Instance, m_NormalModelMatrix));

        m_uiVertexLayout = Backend::CreateVertexLayout();
        m_uiVertexLayout.BindActive();
        m_uiUnit.Init(UI_VERTEX_MAX_COUNT, UI_INDEX_MAX_COUNT, UI_INSTANCE_MAX_COUNT, RENDER_LIMIT_BEHAVIOR);
        const VertexLayoutBindIndex uiVertexBindIndex = m_bufferController.AddVertexBuffer(&m_uiVertexLayout,
            &m_uiUnit.m_VertexBufferHandle, &m_uiUnit.m_IndexBufferHandle);
        vertexAttributes =
        {
            VertexAttribute(0, 2, VertexAttributeBaseType::Float, false, offsetof(VertexUI, m_LocalRectPos)),
            VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(VertexUI, m_UVPos)),
        };
        m_uiVertexLayout.AddAttributes(uiVertexBindIndex, vertexAttributes);
        const VertexLayoutBindIndex uiInstancedBindIndex = m_bufferController.AddVertexBuffer(&m_uiVertexLayout,
            &m_uiUnit.m_InstanceBufferHandle, nullptr);
        instancedAttributes =
        {
            VertexAttribute(2, 4, VertexAttributeBaseType::Float, false, offsetof(InstanceUI, m_Color)),
            VertexAttribute(3, 1, VertexAttributeBaseType::Integer, false, offsetof(InstanceUI, m_TextureIndex)),
            VertexAttribute(4, 1, VertexAttributeBaseType::Float, false, offsetof(InstanceUI, m_Depth)),
        };
        m_uiVertexLayout.AddAttributes(uiInstancedBindIndex, instancedAttributes);
        m_uiVertexLayout.AddMatrixAttribute(Vec2Int(3, 3), uiInstancedBindIndex, 5, false, sizeof(Vec3), offsetof(InstanceUI, m_ModelMatrix));
        

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

        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
        //If we do post process, the color output does not immediately go into the render buffer
        //and must go through pp pass, which means we need our own color output textures
        if (DO_POST_PROCESS)
        {
            //TODO; right now hdr is tightly couples with PP which should can not be this way
            //because if we want pp but dont want hdr there is no corresponding color/depth output that is not hdr textures
            
            m_brightnessOutput = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                                  TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            m_ioTexture = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                           TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            m_hdrDepthRenderBuffer = Backend::CreateRenderBuffer(TexelStorageType::Depth24, windowSize);
        }
        if (DO_RAYTRACING)
        {
            if (RAYTRACE_MODE == RaytraceMode::GPU)
            {
                //NOTE: raytracer on GPU must have textures with rgba16f format (rgb16f not allowed) for image uniforms
                m_texRaytraceAccum0 = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                                       TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
                m_texRaytraceAccum1 = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                                       TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
                m_ColHDR4Output = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                                    TexelStorageType::RGBA16F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            }
            else if (RAYTRACE_MODE == RaytraceMode::CPU)
            {
                //NOTE: we use rgb32f for textures since it is then easier to write to texels on CPU AND
                //since that is the required format for writing to an image so we do it for convenience
                m_texRaytraceAccum0 = CreateTexture(nullptr, windowSize, TextureBufferType::CPU,
                                              TexelStorageType::RGBA32F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
                m_texRaytraceAccum1 = CreateTexture(nullptr, windowSize, TextureBufferType::CPU,
                                              TexelStorageType::RGBA32F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
                //If we have CPU raytracer since we use multithreading, we want safe reads, so buffer is stored on CPU AND
                //on GPU since an output texture for the raytracer must be GPU so it can be used for Post Process
                m_ColHDR4Output = CreateTexture(nullptr, windowSize, TextureBufferType::GPUThreadSafeRead,
                    TexelStorageType::RGBA32F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
            }
        }
        else
        {
            m_ColHDR4Output = CreateTexture(nullptr, windowSize, TextureBufferType::GPU, 
                                                TexelStorageType::RGBA32F, CreateXYZWrapBehavior(WrapBehavior::ClampEdge));
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
    }

    std::uint8_t Renderer::GenerateRuntimeMaterialId() { return m_runtimeMaterialId++; }
    void Renderer::ResetRuntimeMaterialId() { m_runtimeMaterialId = 0; }
    std::filesystem::path Renderer::CreateRaytraceOutputPath()
    {
        return std::string(OUTPUT_RAYTRACE_TEXTURE_PARENT_PATH) +
            (RAYTRACE_MODE == RaytraceMode::CPU ? "cpu" : "gpu") + 
            "_frame_" + std::to_string(m_framesSinceStart) + ".hdr";
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
        if (!::Math::ApproximateEqualsF(material.GetAlpha(), MAX_FLOAT_COLOR_CHANNEL))
            return nullptr;

        if (material.m_Albedo == nullptr)
        {
            LogError(std::format("Attempted to get same draw batch as args with "
                "shader:{} material:{} and vertexCount:{} but material has no albedo assigned", 
                shader.ToString(), material.ToString(), vertexCount));
            return nullptr;
        }
        return m_geometryUnit.TryGetBatch(shader, material.m_Albedo, vertexCount);
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
            m_geometryUnit.TryAddVerticesToBatch(batch, vertexArray, vertexSize);
        }
        if (indicesSize > 0 && indexArray != nullptr)
        {
            m_geometryUnit.TryAddIndicesToBatch(batch, indexArray, indicesSize);
        }
        
        TryAddGeometryInstanceDataToBatch(batch, modelMatrix, material);
        //NOTE: we wait until we have vertices and indices to be able to finish batch
        if (vertexArray != nullptr && indexArray != nullptr) FinishGeometryBatch(batch, blasTree);
        return batch;
    }
    Instance* Renderer::TryAddGeometryInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material)
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
        Instance* createdInstance = m_geometryUnit.TryAddInstanceDataToBatch(batch, 
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
        const BVHNodeStd430& blasTreeRootNode = m_blasTreesAligned[m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval.m_StartIndex];
        m_instanceBoundsData.push_back(InstanceBoundsData(Math::ApplyMatrixToAABB(blasTreeRootNode.GetAABB(), instance.m_ModelMatrix), instanceIndex));
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

        const size_t blasNodeIndexOffset = m_blasTreesAligned.size();
        m_blasTreesAligned.reserve(m_blasTreesAligned.size() + batchBLASTree.Size());
        for (size_t i=0; i< batchBLASTree.Size(); i++)
        {
            const BVHNode& node = batchBLASTree.GetNodes()[i];
            m_blasTreesAligned.push_back(ConvertAlignment<BVHNode::GetAlignment(), BVHNodeStd430::GetAlignment()>(node));
            //NOTE: since the blas tree nodes used LOCAL INDICES
            //into a given index array segment, and it thus needs to be adapted into the global index array by
            //getting an offset of BATCH START INDEX / 3 (because blas object node indices are TRIANGLE INDICES not any indices)
            if (m_blasTreesAligned.back().IsLeaf()) m_blasTreesAligned.back().m_ObjectStartIndex += batch.m_IndicesStartIndex / 3;
            //NOTE: since the indices are local in terms of the root node of the tree, we add the root nodes
            //distance from the start of the tree to all nodes to adjust it
            else
            {
                m_blasTreesAligned.back().m_IndexChild0 += blasNodeIndexOffset;
                m_blasTreesAligned.back().m_IndexChild1 += blasNodeIndexOffset;
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
        Instance* createdInstance = TryAddGeometryInstanceDataToBatch(batch, modelMatrix, material);
        if (createdInstance == nullptr)
        {
            LogWarning("Attempted to add complete geometry instance to batch, but created instance was null");
            return;
        }
        //If this is NOT the first instance to this batch it means the mesh for this batch must exist (NOTE: the
        //mesh for a whole batch is the same) and we can set it to a previous set instance OTHERWSIE we
        //will have to set the mesh index separately when the batch is fully finished
        createdInstance->m_MeshIndex = m_geometryUnit.m_CpuInstances[batch.m_InstanceStartIndex].m_MeshIndex;
        //The most recent added instance data is the one create world mesh bounds
        AddGeometryInstanceMeshBoundsData(batch.m_InstanceStartIndex + batch.m_InstanceCount);
    }

    RenderBatch& Renderer::CreateUIBatch(Shader& shader, Texture* texture, const VertexUI* vertexArray, const size_t vertexSize,
        const IndexType* indexArray, const size_t indicesSize, const ColHDR4& color, const float depth, const Mat3& modelMatrix)
    {
        ENGINE_ASSERT(vertexArray != nullptr && vertexSize > 0 && indexArray != nullptr && indicesSize > 0,
            "Attempted to create create a UI batch with no vertices and/or indices");

        RenderBatch& batch = m_uiUnit.CreateBatch(shader, texture);
        m_uiUnit.TryAddVerticesToBatch(batch, vertexArray, vertexSize);
        m_uiUnit.TryAddIndicesToBatch(batch, indexArray, indicesSize);

        TryAddUIInstanceDataToBatch(batch, color, texture, depth, modelMatrix);
        m_uiUnit.FinishBatch(batch);
        return batch;
    }
    InstanceUI* Renderer::TryAddUIInstanceDataToBatch(RenderBatch& batch, const ColHDR4& color, Texture* texture, 
        const float depth, const Mat3& modelMatrix)
    {
        //TODO: add support for textures
        return m_uiUnit.TryAddInstanceDataToBatch(batch, color, -1, depth, modelMatrix);
    }
    RenderBatch* Renderer::TryGetSameUIDrawBatch(const Shader& shader, const Texture* texture, std::uint32_t vertexCount)
    {
        return m_uiUnit.TryGetBatch(shader, texture, vertexCount);
    }

    void Renderer::ConstructBLASTree(BVHTriangleTree& tree, const size_t indexStart, const size_t indexSize)
    {
        Rendering::ConstructBVHFromIndices(tree, &m_geometryUnit.m_CpuIndices[indexStart],
            indexSize, m_geometryUnit.GetVertexMemPointer());
    }
    void Renderer::ConstructTLASTree(const bool writeToTlasSSBO)
    {
        if (m_instanceBoundsData.empty())
            return;

        //LogWarning(std::format("ALL instance bounds data: {}", ::Utils::ToStringIterable(m_instanceBoundsData)));
        m_tlasTreeAligned.Construct(&m_instanceBoundsData[0], m_instanceBoundsData.size(), true, 1, 
            BVHSplitAlgorithm::Midpoint, &InstanceBoundsData::GetAABB, &InstanceBoundsData::GetCenter, 
            [](const InstanceBoundsData* boundsPtr, const std::uint32_t* objectIndicesArr, const size_t boundsSize, int intendedStartIndex,
                int& outStartIndex, std::uint32_t& outObjectCount) -> void
            {
                //NOTE: the only reason we can do this is because we know there will be only one 
                //instance at the leaves
                outStartIndex = boundsPtr[intendedStartIndex].m_InstanceIndex;
                outObjectCount = boundsSize;
            });
        if (writeToTlasSSBO) m_tlasTreeStorageBuffer.WriteData(0, decltype(m_tlasTreeAligned)::GetNodeByteSize() * m_tlasTreeAligned.Size(), &m_tlasTreeAligned.GetRoot());

        /*LogWarning(std::format("ALL instance bounds data: {}", ::Utils::ToStringIterable(m_instanceBoundsData)));
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
            return INVALID_TEXTURE_INDEX;

        const size_t currTexCount = m_renderTextures.size();
        for (std::uint8_t i = 0; i < currTexCount; i++)
        {
            if (m_renderTextures[i] == texture)
                return i;
        }
        
        ENGINE_ASSERT(currTexCount < TEXTURE_MAX_COUNT,
            "Attempted to enqueue texture for bind but reached max texture count: {}/{}",
            currTexCount, TEXTURE_MAX_COUNT);

        if (DO_RAYTRACING && RAYTRACE_MODE == RaytraceMode::CPU)
        {
            //TODO: what happens if the texture is already a CPU texture? its a waste to copy it again
            ENGINE_ASSERT(m_cpuBufferTextures.size() == m_renderTextures.size(), "Attempted to add texture to CPU buffer textures due to CPU raytracing "
                "but CPU buffer texture size:{} does not match original texture ptr buffer size:{}", m_cpuBufferTextures.size(), m_renderTextures.size());
            m_cpuBufferTextures.emplace_back();
            m_cpuBufferTextures.back().CopyTexture(*texture, TextureBufferType::CPU);
        }
        //NOTE: even if we need gpu thread safe texture, we STILL add texture in default state
        //so it can be searched for its texture index via its unmutated version

        m_renderTextures.push_back(texture);
        return m_renderTextures.size() - 1;
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
            Camera::CalculateViewMatrix(pos, ENGINE_RIGHT_DIR,      -ENGINE_UP_DIR),
            Camera::CalculateViewMatrix(pos, -ENGINE_RIGHT_DIR,     -ENGINE_UP_DIR),
            Camera::CalculateViewMatrix(pos, ENGINE_UP_DIR,         ENGINE_FORWARD_DIR),
            Camera::CalculateViewMatrix(pos, -ENGINE_UP_DIR,        -ENGINE_FORWARD_DIR),
            Camera::CalculateViewMatrix(pos, ENGINE_FORWARD_DIR,    -ENGINE_UP_DIR),
            Camera::CalculateViewMatrix(pos, -ENGINE_FORWARD_DIR,   -ENGINE_UP_DIR)
        };
        outProjMatrix = Math::Platforms::CalculatePlatformPerspectiveProjMatrix(::Math::RAD_90, 1, nearDistance, farDistance);
    }

    void Renderer::AddExistingMeshCall(Model3d& model, Shader& shader, Material& material, const Mat4& modelMatrix)
    {
        ModelMesh& mesh = model.m_Objects[0].m_Mesh;
        const size_t indexCount = mesh.m_Indices.size();
        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, indexCount);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }
        CreateGeometryBatch(shader, material, &mesh.m_Vertices[0], mesh.m_Vertices.size(),
            &mesh.m_Indices[0], indexCount, modelMatrix, &mesh.m_BLASTree);
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

        AddExistingMeshCall(*boxModel, shader, material, modelMatrix);
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
        const Vec2 textureSize = material.m_Albedo->GetInfo().m_TexelSize.AsFloat();
        //The size in texture pixel coords based on its world size
        const Vec3Int pixelSize = CalculateFaceSizeForTexture(Math::ExtractScaleFromMatrix(modelMatrix),
            material.m_Albedo->GetInfo().m_TexelSize);

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

        AddExistingMeshCall(*sphereModel, shader, material, modelMatrix);
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

                m_geometryUnit.TryAddIndicesToBatch(batch, { aIndex, cIndex, bIndex });
                m_geometryUnit.TryAddIndicesToBatch(batch, { bIndex, cIndex, dIndex });
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
            m_geometryUnit.TryAddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
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
            m_geometryUnit.TryAddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        FinishGeometryBatch(batch, nullptr);
    }
    void Renderer::AddCallPlane3DMultiConstructed(Shader& shader, Material& material, const Mat4& modelMatrix, const Vec2& textureRepeats)
    {
        constexpr size_t TOTAL_INDEX_COUNT = 6;
        constexpr size_t TOTAL_VERTEX_COUNT = 4;

        RenderBatch* sameStatebatch = TryGetSameGeometryDrawBatch(shader, material, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddGeometryCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }

        constexpr Vec2 BASE_SIZE = Vec2(1, 1);
        constexpr Vec2 BASE_EXTENTS = Vec2(BASE_SIZE.m_X / 2.0f, BASE_SIZE.m_Y / 2.0f);
        Vertex vertices[TOTAL_VERTEX_COUNT] = {
            //Bottom Left, Bottom right, top right, top left
            Vertex(Vec3(-BASE_EXTENTS.m_X, 0, -BASE_EXTENTS.m_Y), { 0, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(BASE_EXTENTS.m_X, 0, -BASE_EXTENTS.m_Y), { textureRepeats.m_X, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(BASE_EXTENTS.m_X, 0, BASE_EXTENTS.m_Y), textureRepeats, ENGINE_UP_DIR),
            Vertex(Vec3(-BASE_EXTENTS.m_X, 0, BASE_EXTENTS.m_Y), { 0, textureRepeats.m_Y }, ENGINE_UP_DIR)
        };
        IndexType indices[TOTAL_INDEX_COUNT] = { 0, 1, 2, 2, 3, 0 };

        /*
        Vertex vertices[TOTAL_VERTEX_COUNT] = {
            Vertex(Vec3(-BASE_EXTENTS.m_X, 0, BASE_EXTENTS.m_Y), { 0, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(BASE_EXTENTS.m_X, 0, BASE_EXTENTS.m_Y), { textureRepeats.m_X, 0 }, ENGINE_UP_DIR),
            Vertex(Vec3(BASE_EXTENTS.m_X, 0, -BASE_EXTENTS.m_Y), textureRepeats, ENGINE_UP_DIR),
            Vertex(Vec3(-BASE_EXTENTS.m_X, 0, -BASE_EXTENTS.m_Y), { 0, textureRepeats.m_Y }, ENGINE_UP_DIR)
        };
        IndexType indices[TOTAL_INDEX_COUNT] = { 0, 1, 2, 0, 2, 3 };
        */
        CreateGeometryBatch(shader, material, vertices, TOTAL_VERTEX_COUNT, indices, TOTAL_INDEX_COUNT, modelMatrix, nullptr);
    }
    void Renderer::AddCallPlane3D(Material* material, const Mat4& modelMatrix, const Vec2& textureRepeats)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Plane3d, modelMatrix);

        Shader& baseShader = GetBaseTextureShader();
        Material& baseMaterial = GetMaterialOrDefault(material);
        Model3d* planeModel = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetBasicMeshMutable(BasicMeshType::Plane);
        LogWarning(std::format("Imported plane:{}", planeModel->ToString()));
        //AddCallPlane3DMultiConstructed(baseShader, baseMaterial, modelMatrix, textureRepeats);
        //return;

        if (!USE_CACHED_SHAPE_ASSETS || planeModel == nullptr)
        {
            LogWarning(std::format("[Renderer3D]: Added custom-constructed plane 3D render call due to NULL plane asset"));
            AddCallPlane3DMultiConstructed(baseShader, baseMaterial, modelMatrix, textureRepeats);
            return;
        }

        AddExistingMeshCall(*planeModel, baseShader, baseMaterial, modelMatrix);
    }

    void Renderer::AddCallBox3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallSphere3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Math::Quat& rotation)
    {
        AddCallSphere3DMulti(GetBaseShader(), GetMaterialOrDefault(material), 
            //NOTE: since sphere by default has diameter 1 (radius 0.5), ITS SCALE will ultiamtely be 2 * radius (or diameter)
            Math::CalculateModelMatrix(nullptr, worldPos, Vec3(radius*2), rotation));
    }
    void Renderer::AddCallTextureSphere3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), modelMatrix);
    }
    void Renderer::AddCallTextureSphere3D(Material* material, const WorldPosition3D& worldPos, const float radius, const Math::Quat& rotation)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), 
            Math::CalculateModelMatrix(nullptr, worldPos, Vec3(radius), rotation));
    }

    void Renderer::AddCallTextureBox3D(Material* material, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), modelMatrix);
    }

    void Renderer::AddCallRect2D(const ColHDR4& color, Texture* texture, const float depth, const Mat3& modelMatrix)
    {
        constexpr size_t TOTAL_INDEX_COUNT = 6;
        constexpr size_t TOTAL_VERTEX_COUNT = 4;

        Shader& uiShader = GetCoreShader(CoreShader::UI);
        RenderBatch* sameStatebatch = TryGetSameUIDrawBatch(uiShader, texture, TOTAL_VERTEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            TryAddUIInstanceDataToBatch(*sameStatebatch, color, texture, depth, modelMatrix);
            return;
        }

        VertexUI vertices[TOTAL_VERTEX_COUNT] = {
            //Bottom Left, Bottom right, top right, top left
            VertexUI(Vec2(0,0), UV{0, 0}),
            VertexUI(Vec2(1,0), UV{1, 0}),
            VertexUI(Vec2(1,1), UV{1, 1,}),
            VertexUI(Vec2(0,1), UV{0, 1})
        };
        IndexType indices[TOTAL_INDEX_COUNT] = { 0, 1, 2, 2, 3, 0 };
        
        CreateUIBatch(uiShader, texture, vertices, TOTAL_VERTEX_COUNT, indices, TOTAL_INDEX_COUNT, color, depth, modelMatrix);
    }

    void Renderer::AddCallPointLight(const WorldPosition3D& worldPos, const Math::Quat& worldRot, const float radius, const ColHDR4& color)
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
                Math::CalculateModelMatrix(nullptr, worldPos, std::min(0.1f * radius, 1.0f), Math::Quat::Identity()));
        }
    }
    void Renderer::SetDirectionalLight(const Vec3& dir, const ColHDR4& color)
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

                AddCallMesh(obj->m_Mesh, obj->m_MaterialAsset->GetMaterialMutable(), modelMatrix * meshGroup.m_GlobalTransform);
            }
        }
    }
    void Renderer::AddCallMesh(const ModelMesh& mesh, Material& material, const Mat4& modelMatrix)
    {
        m_frameGeometryMetrics.m_RenderCallInvocations.emplace_back(RenderCallType::Model3d, modelMatrix);
        CreateGeometryBatch(GetBaseTextureShader(), material, &(mesh.m_Vertices[0]), mesh.m_Vertices.size(),
            &(mesh.m_Indices[0]), mesh.m_Indices.size(), modelMatrix, &mesh.m_BLASTree);
    }

    void Renderer::AddCallAABBWifreframe(const Mat4& modelMatrix, const ColHDR4& color, const float lineThickness)
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

        const Camera::CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
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
            cameraLineOrthogonal = Math::CrossProduct(cameraDir, (vertex1 - vertex0).GetNormalized()).GetNormalized();

            //TODO: FINISH
        }
        FinishGeometryBatch(batch, nullptr);
    }
    void Renderer::AddCallAABBWifreframe(const AABB3D& aabb, const Math::Quat& rotation, const ColHDR4& color, const float lineThickness)
    {
        AddCallAABBWifreframe(Math::CalculateModelMatrix(nullptr, aabb.GetCenter(), aabb.GetSize(), rotation), color, lineThickness);
    }

    void Renderer::AddBVHTreeBoundsWireframe()
    {
        for (const auto& node : m_tlasTreeAligned.GetNodes())
        {
            AddCallAABBWifreframe(node.GetAABB(), Math::Quat::Identity(),
                node.IsLeaf()? BVH_BOUNDS_LEAF_COLOR : BVH_BOUNDS_COLOR, BVH_BOUNDS_LINE_THICKNESS);
        }
    }
    bool Renderer::IntersectsBVH(Math::Ray3D rayWorld, const Vertex* outHitVertex)
    {
        rayWorld.m_Dir.Normalize();

        IndexTriangle* trianglePtr = reinterpret_cast<IndexTriangle*>(&m_geometryUnit.m_CpuIndices[0]);
        return m_tlasTreeAligned.Intersects<Instance>(rayWorld, &m_geometryUnit.m_CpuInstances[0], nullptr, nullptr,
            [this, trianglePtr](const BVHNodeStd430& node, const Instance& instance,
                const Math::Ray3D& rayWorld, float* outTopHitDistance) -> bool
            {
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                const WorldPosition3D rayLocalOrigin = (instance.m_InverseModelMatrix * Vec4(rayWorld.m_Origin, 1)).GetXYZ();
                const WorldPosition3D rayLocalDir = (instance.m_InverseModelMatrix * Vec4(rayWorld.m_Dir, 0)).GetXYZ().GetNormalized();
                /*
                LogWarning(std::format("Inverse mat:{} rayO {}->{} rayDir {}->{}", instance.m_InverseModelMatrix.ToString(), 
                    rayOrigin.ToString(), rayLocalOrigin.ToString(), rayDir.ToString(), rayLocalDir.ToString()));
                LogWarning(std::format("Ray (LOCAL) {} -> {} reached blas level (LOCAL) area: {} SHOULD INTERSECT:{}", 
                    rayLocalOrigin.ToString(), rayLocalDir.ToString(),
                    ::Utils::ApplyMatrixToAABB(node.GetAABB(), instance.m_InverseModelMatrix).ToString(), 
                    ::Utils::RayIntersectsSphere(Vec3(), 0.2, rayLocalOrigin, rayLocalDir, nullptr)));
                    */
                return Engine::IntersectsBVH<IndexTriangle, STD_430_ALIGN>(Math::Ray3D{ rayLocalOrigin, rayLocalDir }, 
                    &m_blasTreesAligned[treeInterval.m_StartIndex],
                    treeInterval.m_Size, trianglePtr, nullptr, nullptr, nullptr,
                    [this, outTopHitDistance, &instance, rayWorld](const BVHNodeStd430& node, const IndexTriangle& triangle,
                        const Math::Ray3D& rayLocal, float* outBottomHitDistance) -> bool
                    {
                        /*
                        LogWarning(std::format("Ray {} -> {} reached vertex level with triangle: {} {} {}", rayLocalOrigin.ToString(), rayLocalDir.ToString(),
                            m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex0].m_LocalPos.ToString(),
                            m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex1].m_LocalPos.ToString(), m_geometryUnit.m_CpuVertices[triangle.m_VertexIndex2].m_LocalPos.ToString()));
                        */

                        float outTEnter = 0;
                        const bool intersectsTriangle = Math::RayIntersectsTriangle(m_geometryUnit.m_CpuVertices[triangle.m_0].m_LocalPos,
                            m_geometryUnit.m_CpuVertices[triangle.m_1].m_LocalPos, m_geometryUnit.m_CpuVertices[triangle.m_2].m_LocalPos, rayLocal, &outTEnter);
                        *outBottomHitDistance = (rayLocal.m_Dir * outTEnter).GetMagnitude();
                        const Vec3 worldHitPos = (instance.m_ModelMatrix * Vec4(rayLocal.m_Origin + rayLocal.m_Dir * outTEnter, 1)).GetXYZ();
                        *outTopHitDistance = (worldHitPos - rayWorld.m_Origin).GetMagnitude();
                        return intersectsTriangle;
                    });
            });
    }
    bool Renderer::IsValidBVH()
    {
        //NOTE: this should ONLY be called after TLAS tree has been constructed and some blas nodes are added
        IndexTriangle* trianglePtr = reinterpret_cast<IndexTriangle*>(&m_geometryUnit.m_CpuIndices[0]);
        return m_tlasTreeAligned.IsValid<Instance>(&m_geometryUnit.m_CpuInstances[0],
            //Override getBounds of BLAS leaf node primitives (InstanceType)
            [this](const Instance& instance) -> AABB3D
            {
                //NOTE: since we have change the object indices in the TLAS to be indices into instances,
                //and since each leaf in the TLAS has only 1 INSTANCE, we can just get the roots BLAS tree aabb
                //which should be the same as the object aabb
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                return Math::ApplyMatrixToAABB<STD_430_ALIGN, AABB3D::GetAlignment()>(
                    m_blasTreesAligned[treeInterval.m_StartIndex].m_Bounds, instance.m_ModelMatrix);
            },
            //TLAS leaf successor is valid function
            [this, trianglePtr](const BVHNodeStd430& leafNode) -> bool
            {
                //NOTE: the object indices of TLAS tree are indices into instances
                const Instance& instance = m_geometryUnit.m_CpuInstances[leafNode.m_ObjectStartIndex];
                const ArrayInterval treeInterval = m_instanceMeshes[instance.m_MeshIndex].m_BLASTreesInterval;
                return Engine::IsValidBVH<IndexTriangle, STD_430_ALIGN>(&m_blasTreesAligned[treeInterval.m_StartIndex], m_geometryUnit.m_CpuIndices.size() / 3, trianglePtr, nullptr,
                    [this](const IndexTriangle& triangle) -> AABB3D
                    {
                        //NOTE: this ONLY WORKS IF WE APPLIED OBEJCT LEAF NODE INDEX OFFSET TO BLAS TREES
                        //SO THEY INDEX INTO GLOBAL INDEX ARRAY AND NOT JUST LOCAL MESH ARRAY
                        return CalculateTriangleAABB(triangle, &m_geometryUnit.m_CpuVertices[0]);
                    }, nullptr, true);
            }, true);
    }

    void Renderer::SetViewUniformBuffer(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix)
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
    void Renderer::SetViewUniformBuffer(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix,
        const Vec3& forwardDir, const Vec3& rightDir, const Vec3& upDir, const float yFov)
    {
        SetViewUniformBuffer(worldPos, viewMatrix, projMatrix);
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
    void Renderer::UpdateLightAndViewerBlock(const bool updateUniformBuffers)
    {
        if (m_uniformData.m_LightBufferNeedsUpdate)
        {
            if (updateUniformBuffers) m_lightUniformBuffer.WriteData(0, sizeof(LightBlockData), &m_uniformData.m_LightBlock);
            m_uniformData.m_LightBufferNeedsUpdate = false;
        }
        //TODO: right now camera data is always written FIX THIS
        const Camera::CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const Camera::CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        camera.GetTransform().CalculateWorldDirections(&m_uniformData.m_ViewerBlock.m_FowardDir, 
            &m_uniformData.m_ViewerBlock.m_UpDir, &m_uniformData.m_ViewerBlock.m_RightDir);

        m_uniformData.m_ViewerBlock.m_WorldPos = camera.GetTransform().GetWorldPos();
        m_uniformData.m_ViewerBlock.m_ViewMatrix = cameraData.m_ViewMatrix;
        m_uniformData.m_ViewerBlock.m_ProjectionMatrix = cameraData.m_PlatformProjectionMatrix;
        m_uniformData.m_ViewerBlock.m_FovY = camera.GetSettings().m_FieldOfViewYRadians;

        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
        m_uniformData.m_ViewerBlock.m_ScreenSize = windowSize;

        if (updateUniformBuffers)
        {
            SetViewUniformBuffer(m_uniformData.m_ViewerBlock.m_WorldPos, m_uniformData.m_ViewerBlock.m_ViewMatrix,
                m_uniformData.m_ViewerBlock.m_ProjectionMatrix, m_uniformData.m_ViewerBlock.m_FowardDir,
                m_uniformData.m_ViewerBlock.m_RightDir, m_uniformData.m_ViewerBlock.m_UpDir,
                m_uniformData.m_ViewerBlock.m_FovY);
        }
        
        if (::Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, Camera::CameraPrecalculatedDataUpdate::ViewMatrix))
        {
            m_unmovingFrames = 0;
        }
    }

    void Renderer::DrawIndexedInstancedBatch(RenderBatch& batch, VertexLayout& bindLayout)
    {
        if (!bindLayout.IsBoundActive())
            bindLayout.BindActive();

        Backend::DrawUploadedIndexBufferInstanced(0,
            batch.m_VertexStartIndex * sizeof(IndexType),
            batch.m_IndicesCount, batch.m_InstanceStartIndex, batch.m_InstanceCount);
    }
    void Renderer::SetSkybox(Texture* texture)
    {
        //NOTE: if we have CPU raytracer we need to ensure the skybox pointer
        //refers to one with a CPU buffer
       
        //TODO: this is very messy and bad we could instead force texture enqueue even if the xkybox
        //is for non raytrace or CPU raytrace and then just use index into texture buffer like we do 
        //with materials for texture when invoking shaders
        if (texture != nullptr && DO_RAYTRACING && RAYTRACE_MODE == RaytraceMode::CPU)
        {
            int skyboxTexIndex = GetEnqueuedTextureIndex(texture);
            m_skybox = &m_cpuBufferTextures[skyboxTexIndex];
        }
        else m_skybox = texture;
    }

    void Renderer::ExecuteSkyboxPass(Texture& outputTexture, std::uint8_t* outDrawnAttachmentsMask)
    {
        UpdatePassRenderState(RenderPassType::Skybox);
        m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &outputTexture);

        //NOTE: we only need to clear the background color and not depth since we ignore depth
        Backend::ClearBackground(0b1);
        if (outDrawnAttachmentsMask != nullptr)
            *outDrawnAttachmentsMask |= 0b1;

        Shader& skyboxShader = GetCoreShader(CoreShader::Skybox);
        BindShader(skyboxShader);

        SlotIndex slot = m_textureController.TryBindToFreeSlot<Texture>(*m_skybox);
        skyboxShader.TrySetUniform(UniformDataType::Sampler2D, SKYBOX_UNIFORM_NAME, &slot);

        const Camera::CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const Camera::CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        Math::Quat cameraRotation = camera.GetTransform().GetWorldRotation();
        const Vec3 forwardDir = cameraRotation.ApplyRotationToDir(ENGINE_FORWARD_DIR);
        const Vec3 rightDir = Math::CrossProduct(ENGINE_UP_DIR, forwardDir).GetNormalized();
        Mat4 noTranslationViewMatrix = Math::CalculateModelMatrix(nullptr, Vec3::Zero(), Vec3::One(),
            Math::CalculateRotationMatrix(forwardDir, ENGINE_UP_DIR, rightDir));
        //SetViewerData(camera.GetTransform().GetWorldPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix);
        SetViewUniformBuffer(camera.GetTransform().GetWorldPos(), noTranslationViewMatrix, cameraData.m_PlatformProjectionMatrix);

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
            Backend::SetViewport(m_shadowMaps[i].GetData().m_TexelSize.m_X, m_shadowMaps[i].GetData().m_TexelSize.m_Y);

            //For every single face on cube, we redraw scene from light perspective
            for (size_t j = 0; j < 6; j++)
            {
                m_frameBuffer.SetOutputTextureCube(FrameBufferAttachmentType::Depth, &m_shadowMaps[i], static_cast<TextureCubeFace>(j));
                Backend::ClearBufferBit(BufferBitType::Depth);

                SetViewUniformBuffer(light.m_Pos, lightViewMatrices[j], lightProjMatrix);

                for (size_t k = 0; k < m_geometryUnit.m_Batches.size(); k++)
                {
                    auto& batch = m_geometryUnit.m_Batches[k]; 
                    if (batch.m_InstanceCount == 0)
                        continue;

                    DrawIndexedInstancedBatch(batch, m_geometryVertexLayout);
                }
            }
        }
   
        UnbindActiveShader();
        //This forces the viewport to be set back to rendering for the window
        m_engineState->m_GraphicsContext.m_Window->ForceSizeUpdate();
    }
    void Renderer::ExecuteLightingAndGeometryPass(Texture& outputTexture, const SlotIndex* shadowCubeMapSlots, const std::uint8_t previousDrawnColorAttachmentsMask)
    {
        UpdatePassRenderState(RenderPassType::Geometry);
        m_boundFrameBuffer->SetOutputRenderBuffer(FrameBufferAttachmentType::Depth, &m_hdrDepthRenderBuffer);
        m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &outputTexture);
        m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color1, &m_brightnessOutput);

        //If we did not draw to color attachments before this, we clear the two colors attachments we will need, color0 and color1
        //to ensure we do not have previous frame color data here
        const std::uint8_t clearBitmask = 0b11;
        if (previousDrawnColorAttachmentsMask == 0) Backend::ClearBackground(clearBitmask);
        //If the previous draw mask is not 0, we want to ignore any bits it drew previously but we dont want to clear now
        //(so 0 bit in clear makes any bit 0 even if previousMask is 0 or 1 AND any 1 bit in clear stays 1 unless previous is also 1
        //so we only clear necessary slots that we need right now, but only if they were not previouslt drawn to)
        else Backend::ClearBackground(~(~clearBitmask | previousDrawnColorAttachmentsMask));
        UpdateLightAndViewerBlock(true);

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

            ENGINE_ASSERT(batch.m_Shader != nullptr, "Tried to flush Lighting/Geometry batch in renderer, but batch shader was null");

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
            DrawIndexedInstancedBatch(batch, m_geometryVertexLayout);
        }

        if (lastBatchShader != nullptr) unbindLastBatchShader();
        if (lastBatchTexture != nullptr) removeLastBatchTexture();

        //LogWarning(std::format("LIGHT PASS Texture controler after pass: {}", m_textureController.ToString()));
        //LogError(std::format("HDR color texture: {}", ::Utils::ToStringMemory(writePtr, byteSize)));
    } 

    Texture& Renderer::ExecuteForwardRendering()
    {
        const bool hasSkybox = m_skybox != nullptr;
        std::uint8_t previousDrawnAttachmentsMask = 0;
        Texture& colorOutputTex = m_ColHDR4Output;
        if (hasSkybox) ExecuteSkyboxPass(colorOutputTex, &previousDrawnAttachmentsMask);

        //NOTE: we do this to ensure that we only add any data as long as all 3 buffers have enough space
        const auto& freeVertexSeg = m_geometryUnit.m_VertexBufferHandle.TryGetFreeSegment(m_geometryUnit.GetVertexCount());
        const auto& freeIndexSeg = m_geometryUnit.m_IndexBufferHandle.TryGetFreeSegment(m_geometryUnit.GetIndexCount());
        const auto& freeInstanceSeg = m_geometryUnit.m_InstanceBufferHandle.TryGetFreeSegment(m_geometryUnit.GetInstanceCount());
        if (freeVertexSeg == std::nullopt || freeIndexSeg == std::nullopt || freeInstanceSeg == std::nullopt)
        {
            m_isRenderStalled = true;
            const std::string message = std::format("Stalling FORWARD RENDERING vertex:{} index:{} instance:{}",
                freeVertexSeg == std::nullopt, freeIndexSeg == std::nullopt, freeIndexSeg == std::nullopt);
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return colorOutputTex;
        }

        FencedBufferSegment& vertexFenceSeg = m_geometryUnit.m_VertexBufferHandle.WriteDataFenced(
                                              m_geometryUnit.GetVertexMemPointer(), freeVertexSeg.value());
        FencedBufferSegment& indexFenceSeg = m_geometryUnit.m_IndexBufferHandle.WriteDataFenced(
                                             m_geometryUnit.GetIndexMemPointer(), freeIndexSeg.value());
        FencedBufferSegment& instanceFenceSeg = m_geometryUnit.m_InstanceBufferHandle.WriteDataFenced(
                                                m_geometryUnit.GetInstanceMemPointer(), freeInstanceSeg.value());

        std::vector<SlotIndex> shadowCubeMapSlots = {};
        if (DO_SHADOWS)
        {
            ExecuteShadowPass();

            vertexFenceSeg.m_Fence.Insert();
            indexFenceSeg.m_Fence.Insert();
            instanceFenceSeg.m_Fence.Insert();

            const size_t totalPointLights = m_uniformData.m_LightBlock.m_PointLightsCount;
            shadowCubeMapSlots = m_textureController.TryBindToFreeSlots<TextureCube>(m_shadowMaps, totalPointLights);
            ENGINE_ASSERT(!shadowCubeMapSlots.empty() && shadowCubeMapSlots.size() == totalPointLights, 
                "Attempted to add shadow map textures to available slots but failed."
                "Reserved slots:{} expected size:{}", shadowCubeMapSlots.size(), totalPointLights);
        }

        ExecuteLightingAndGeometryPass(colorOutputTex, DO_SHADOWS ? &shadowCubeMapSlots[0] : nullptr, previousDrawnAttachmentsMask);
        if (DO_SHADOWS) m_textureController.RemoveFromSlots(shadowCubeMapSlots);
        else
        {
            vertexFenceSeg.m_Fence.Insert();
            indexFenceSeg.m_Fence.Insert();
            instanceFenceSeg.m_Fence.Insert();
        }
        return colorOutputTex;
    }

    void Renderer::ExecuteUIPass(Texture& textureInputOutput)
    {
        UpdatePassRenderState(RenderPassType::UI);

        //NOTE: we do this to ensure that we only add any data as long as all 3 buffers have enough space
        const auto& freeVertexSeg = m_uiUnit.m_VertexBufferHandle.TryGetFreeSegment(m_uiUnit.GetVertexCount());
        const auto& freeIndexSeg = m_uiUnit.m_IndexBufferHandle.TryGetFreeSegment(m_uiUnit.GetIndexCount());
        const auto& freeInstanceSeg = m_uiUnit.m_InstanceBufferHandle.TryGetFreeSegment(m_uiUnit.GetInstanceCount());
        if (freeVertexSeg == std::nullopt || freeIndexSeg == std::nullopt || freeInstanceSeg == std::nullopt)
        {
            m_isRenderStalled = true;
            std::string message = std::format("Stalling UI PASS vertex:{} index:{} instance:{}",
                freeVertexSeg == std::nullopt, freeIndexSeg == std::nullopt, freeInstanceSeg == std::nullopt);

            if (freeVertexSeg == std::nullopt) message += std::format("Vertices needed:{} Allocated:{}", 
                m_uiUnit.GetVertexCount(), m_uiUnit.m_VertexBufferHandle.GetAllocatedByteSize() / sizeof(VertexUI));
            if (freeIndexSeg == std::nullopt) message += std::format("Indices needed:{} Allocated:{}",
                m_uiUnit.GetIndexCount(), m_uiUnit.m_IndexBufferHandle.GetAllocatedByteSize() / sizeof(IndexType));
            if (freeInstanceSeg == std::nullopt) message += std::format("Instances needed:{} Allocated:{}",
                m_uiUnit.GetInstanceCount(), m_uiUnit.m_InstanceBufferHandle.GetAllocatedByteSize() / sizeof(InstanceUI));

            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }
        FencedBufferSegment& vertexFenceSeg = m_uiUnit.m_VertexBufferHandle.WriteDataFenced(
            m_uiUnit.GetVertexMemPointer(), freeVertexSeg.value());
        FencedBufferSegment& indexFenceSeg = m_uiUnit.m_IndexBufferHandle.WriteDataFenced(
            m_uiUnit.GetIndexMemPointer(), freeIndexSeg.value());
        FencedBufferSegment& instanceFenceSeg = m_uiUnit.m_InstanceBufferHandle.WriteDataFenced(
            m_uiUnit.GetInstanceMemPointer(), freeInstanceSeg.value());

        //The texture that is input is the one that has UI drawn over
        m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &textureInputOutput);

        //Here we clear the depth ONLY since we do not want old depth but we still need it for ui draw layers
        //NOTE: we are guaranteed to have written to color0 either via raytracer or forward renderer
        //so we do not clear it, but we still use it to override the colors on top
        Backend::ClearBackground(0b0);

        Shader* lastBatchShader = nullptr;
        const auto unbindLastBatchShader = [this, &lastBatchShader]() -> void
            {
                UnbindActiveShader();
                lastBatchShader = nullptr;
            };

        for (int i = 0; i < m_uiUnit.m_Batches.size(); i++)
        {
            auto& batch = m_uiUnit.m_Batches[i];
            if (batch.m_InstanceCount == 0)
                continue;

            ENGINE_ASSERT(batch.m_Shader != nullptr, "Tried to flush UI batch in renderer, but batch shader was null");
            std::vector<InstanceUI> instances = {};
            m_uiUnit.m_InstanceBufferHandle.ReadDataAs<InstanceUI>(instances, true);
            //LogError(std::format("UI instances: {}", ::Utils::ToStringIterable(instances)));
            //LogWarning(std::format("Has indices:{}", batch.m_IndicesCount));

            if (lastBatchShader != nullptr && batch.m_Shader == nullptr) unbindLastBatchShader();
            else if (lastBatchShader == nullptr || lastBatchShader != batch.m_Shader)
            {
                BindShader(*batch.m_Shader);
            }
            lastBatchShader = batch.m_Shader;
            DrawIndexedInstancedBatch(batch, m_uiVertexLayout);
        }

        if (lastBatchShader != nullptr) unbindLastBatchShader();
    }
    void Renderer::ExecutePostProcessPass(Texture& noPPColorOutputTex)
    {
        UpdatePassRenderState(RenderPassType::PostProcess);

        if (DO_BLOOM)
        {
            ApplyBlurInPlace(m_brightnessOutput, m_ioTexture, 2);
        }

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
        const SlotIndex hdrOutputIndex = m_textureController.TryBindToFreeSlot<Texture>(noPPColorOutputTex);
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

        if (inputTexture.GetInfo().m_TexelSize != tempTexture.GetInfo().m_TexelSize)
        {
            LogError(std::format("Attempted to apply blur for texture:{} to output:{} "
                "but they have different texel sizes", inputTexture.ToString(), tempTexture.ToString()));
            return;
        }
        if (tempTexture.GetInfo().m_InternalStorage != TexelStorageType::RGBA16F)
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
        const Vec3Int computeGroups = Vec3Int(inputTexture.GetInfo().m_TexelSize, 1);
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

    Texture& Renderer::ExecuteRayTracing(Texture* overrideOutputTexture)
    {
        UpdatePassRenderState(RenderPassType::RayTrace);

        //NOTE: since gpu stalls dont occur due to raytracer, we should always have even/odd alternating frames
        //since then with ping-ponging still get accumulation logic without needing to write back output to input texture
        Texture& inputTexture = (m_framesSinceStart & 1) ? m_texRaytraceAccum0 : m_texRaytraceAccum1;
        Texture& outputTexture = (m_framesSinceStart & 1) ? m_texRaytraceAccum1 : m_texRaytraceAccum0;
        //If we do not have any instances, the raytracers would do pointless extra work
        if (m_geometryUnit.m_CpuInstances.size() != 0)
        {
            Texture* resultTex = nullptr;
            if (RAYTRACE_MODE == RaytraceMode::GPU)
                resultTex = &ExecuteRayTracingGPU(inputTexture, outputTexture, overrideOutputTexture);
            else
                resultTex= &ExecuteRayTracingCPU(inputTexture, outputTexture, overrideOutputTexture);

            ENGINE_ASSERT(resultTex != nullptr, "Executed raytracing but texture result is NULL");
            if (OUTPUT_RAYTRACE_FRAME >= 0 && m_framesSinceStart == OUTPUT_RAYTRACE_FRAME)
                WriteTextureToFile(CreateRaytraceOutputPath(), *resultTex);

            return *resultTex;
        }

        return overrideOutputTexture != nullptr ? *overrideOutputTexture : outputTexture;
    }
    Texture& Renderer::ExecuteRayTracingGPU(Texture& inputTex, Texture& outputTex, Texture* overrideOutputTexture)
    {
        Shader& rayTraceShader = GetCoreShader(CoreShader::RayTrace);
        rayTraceShader.BindActive();

        //TODO: this should get updated to include the first raytrace invocations because due to 0 instances, or stalling, it may not be
        //the first frame and none of this stuff would get called
        if (m_framesSinceStart == 0)
        {
            rayTraceShader.TrySetUniform(UniformDataType::Uint, RAY_TRACING_MAX_RAY_BOUNCES_UNIFORM_NAME, &MAX_RAYTRACE_BOUNCES);
            rayTraceShader.TrySetUniform(UniformDataType::Uint, RAY_TRACING_SAMPLES_PER_PIXEL_UNIFORM_NAME, &RAYTRACER_SAMPLES_PER_PIXEL);

            constexpr std::uint32_t RAYTRACER_OUTPUT_TEXTURE_COUNT = DRAW_UI ? 2 : 1;
            rayTraceShader.TrySetUniform(UniformDataType::Uint, RAYTRACER_OUTPUT_TEXTURE_COUNT_UNIFORM_NAME, &RAYTRACER_OUTPUT_TEXTURE_COUNT);
        }
        //If we dont do static goemetry we write every frame, otherwise
        //we only write during the first geometry init
        const std::uint32_t emissiveMaterialCount = m_emissiveInstanceIndices.size();
        if (!DO_STATIC_GEOMETRY || (DO_STATIC_GEOMETRY && m_framesSinceStart == 0))
        {
            m_vertexStorageBuffer.WriteData(0, m_geometryUnit.GetVertexCount() * sizeof(Vertex), m_geometryUnit.GetVertexMemPointer());
            m_indexStorageBuffer.WriteData(0, m_geometryUnit.GetIndexCount() * sizeof(IndexType), m_geometryUnit.GetIndexMemPointer());
            m_instanceStorageBuffer.WriteData(0, m_geometryUnit.GetInstanceCount() * sizeof(Instance), m_geometryUnit.GetInstanceMemPointer());
            m_materialStorageBuffer.WriteData(0, m_materialData.size() * sizeof(MaterialData), &m_materialData[0]);
            /*LogWarning(std::format("Instance mesh buffer elements:{} write:{} bytes:{}", m_instanceMeshStorageBuffer.GetAllocatedByteSize() / sizeof(InstanceMesh),
                m_instanceMeshes.size(), m_instanceMeshes.size() * sizeof(InstanceMesh)));*/
            m_instanceMeshStorageBuffer.WriteData(0, m_instanceMeshes.size() * sizeof(InstanceMesh), &m_instanceMeshes[0]);
            if (emissiveMaterialCount > 0)
            {
                m_emissiveInstanceIndexStorageBuffer.WriteData(0,
                    emissiveMaterialCount * sizeof(std::uint32_t), &m_emissiveInstanceIndices[0]);
            }

            m_blasTreesStorageBuffer.WriteData(0, sizeof(decltype(m_blasTreesAligned[0])) * m_blasTreesAligned.size(), &m_blasTreesAligned[0]);
        }
        rayTraceShader.TrySetUniform(UniformDataType::Uint, EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME, &emissiveMaterialCount);

        const SlotIndex inputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(inputTex, AccessPermissions::Read);
        const SlotIndex outputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(outputTex, AccessPermissions::Write);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, INPUT_TEXTURE_UNIFORM_NAME, &inputTextureSlot);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, OUTPUT_TEXTURE_0_UNIFORM_NAME, &outputTextureSlot);

        SlotIndex overrideOutputTextureSlot = INVALID_SLOT_INDEX;
        if (overrideOutputTexture != nullptr)
        {
            overrideOutputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(*overrideOutputTexture, AccessPermissions::Write);
            rayTraceShader.TrySetUniform(UniformDataType::Image2D, OUTPUT_TEXTURE_1_UNIFORM_NAME, &overrideOutputTextureSlot);
        }
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

        int* textureSampleSlots = nullptr;
        if (!m_renderTextures.empty())
        {
            textureSampleSlots = (int*)alloca(sizeof(int) * m_renderTextures.size());
            for (std::uint8_t i = 0; i < m_renderTextures.size(); i++)
            {
                SlotIndex slotIndex = m_textureController.TryBindToFreeSlot<Texture>(*m_renderTextures[i]);
                ENGINE_ASSERT(slotIndex != INVALID_SLOT_INDEX, "Failed to bind slot index");
                textureSampleSlots[i] = slotIndex;
            }

            rayTraceShader.TrySetUniformArray(UniformDataType::Sampler2D,
                TEXTURES_UNIFORM_NAME, textureSampleSlots, m_renderTextures.size());
        }

        UpdateLightAndViewerBlock(true);
        rayTraceShader.TrySetUniform(UniformDataType::Uint, UNMOVING_FRAME_NUMBER_UNIFORM_NAME, &m_unmovingFrames);

        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();
        rayTraceShader.DispatchComputeShaderGroups(Vec3Int(windowSize, 1));

        //We must invoke memory sync to ensure image operation applied to OUTPUT texture go through before
        //using it for applying it to the hdr color output
        Backend::InvokeImageMemorySync(ImageOperationBarrierType::TextureFetch);
        rayTraceShader.UnbindActive();

        m_imageController.TryRemoveFromSlot(inputTextureSlot);
        if (outputTextureSlot != inputTextureSlot) m_imageController.TryRemoveFromSlot(outputTextureSlot);
        if (bloomSlot != INVALID_SLOT_INDEX) m_imageController.TryRemoveFromSlot(bloomSlot);
        if (skyboxSlot != INVALID_SLOT_INDEX) m_textureController.TryRemoveFromSlot(skyboxSlot);

        for (std::uint8_t i = 0; i < m_renderTextures.size(); i++)
        {
            m_textureController.TryRemoveFromSlot(textureSampleSlots[i]);
        }
        return overrideOutputTexture != nullptr ? *overrideOutputTexture : outputTex;
    }
    Texture& Renderer::ExecuteRayTracingCPU(Texture& inputTex, Texture& outputTex, Texture* overrideOutputTexture)
    {
        //TODO: this should get updated to include the first raytrace invocations because due to 0 instances, or stalling, it may not be
        //the first frame and none of this stuff would get called
        if (m_framesSinceStart == 0)
        {
            m_raytracer.m_Settings.m_MaxBounces = MAX_RAYTRACE_BOUNCES;
            m_raytracer.m_Settings.m_SamplesPerPixel = RAYTRACER_SAMPLES_PER_PIXEL;
            m_raytracer.m_Settings.m_BloomThreshold = BLOOM_THRESHOLD;

            //Since we use references for block, we dont need to update them every frame since only the data inside the block updates
            m_raytracer.m_LightBlock = &m_uniformData.m_LightBlock;
            m_raytracer.m_ViewBlock = &m_uniformData.m_ViewerBlock;
        }
        if (!DO_STATIC_GEOMETRY || (DO_STATIC_GEOMETRY && m_framesSinceStart == 0))
        {
            m_raytracer.m_Vertices = m_geometryUnit.GetVertexMemPointer();
            m_raytracer.m_Indices = m_geometryUnit.GetIndexMemPointer();
            m_raytracer.m_Instances = m_geometryUnit.GetInstanceMemPointer();
            m_raytracer.m_Meshes = &m_instanceMeshes[0];
            m_raytracer.m_Materials = &m_materialData[0];
            if (m_cpuBufferTextures.size() > 0) m_raytracer.m_Textures = &m_cpuBufferTextures[0];
            //Since raytrace does NOT get invoked with 0 isntaces -> we have at least 1 node in tlas and blas trees
            //and since they get updated BEFORE the raytracer, these should be guaranteed non-null
            m_raytracer.m_BlasNodes = &m_blasTreesAligned[0];
            m_raytracer.m_TlasNodes = &m_tlasTreeAligned.GetNodes()[0];

            const std::uint32_t emissiveMaterialCount = m_emissiveInstanceIndices.size();
            m_raytracer.m_Settings.m_EmissiveCount = emissiveMaterialCount;
            if (emissiveMaterialCount > 0) m_raytracer.m_EmissiveInstanceIndices = &m_emissiveInstanceIndices[0];
        }

        m_raytracer.m_InputTex = &inputTex;
        m_raytracer.m_OutputTex0 = &outputTex;
        if (overrideOutputTexture != nullptr)
            m_raytracer.m_OutputTex1 = overrideOutputTexture;

        if (DO_BLOOM)
            m_raytracer.m_BrightnessTex = &m_brightnessOutput;
        if (m_skybox != nullptr)
            m_raytracer.m_SkyboxTex = m_skybox;

        UpdateLightAndViewerBlock(false);
        m_raytracer.m_Settings.m_UnmovingFrameCount = m_unmovingFrames;

        m_raytracer.Run();
        //NOTE: since the output texture in the raytracer is then used for Post Process, the output texture must be 
        //a gpu texture (which the accum0, accum1 ARE NOT) so we write the output from CPU textures to a GPU texture
        m_ColHDR4Output.CopyBytes(outputTex);
        //LogWarning(std::format("hdr tex Color outpit:{}", m_ColHDR4Output.ToStringBytes(false)));
        //LogError(std::format("is emtpy copy:{}", m_ColHDR4Output.HasEmptyData()));
        return m_ColHDR4Output;
    }

    void Renderer::FlushBatches()
    {
        if (RENDER_FRAMES_COUNT != NO_RENDER_FRAME_COUNT_LIMIT &&
            m_framesSinceStart >= RENDER_FRAMES_COUNT)
        {
            LogError(std::format("Reached render frame count of: {}", m_framesSinceStart));
            return;
        }
        
        //TODO: if we add more instances and have non0static geometry, tlas tree needs to get updated too
        //Or if objects move
        if (m_framesSinceStart == 0)
        {
            //We only write to SSBBO if we are raytracing for the GPU
            ConstructTLASTree(DO_RAYTRACING && RAYTRACE_MODE == RaytraceMode::GPU);
        }
        SetSkybox(m_graphicsManager->GetSkyboxMutable());
            
        Texture* texOutput = nullptr;
        if (DO_RAYTRACING) 
        {
            //If we draw UI with raytracer, we can NOT just use accumulation frame output to then
            //get overdrawn by UI and then used in the subsequent frame so in that case we use a custom
            //ui output texture
            if (DRAW_UI)
            {
                ExecuteRayTracing(&m_texUIOutput);
                texOutput = &m_texUIOutput;
            }
            else texOutput = &ExecuteRayTracing(nullptr);
        }
        //Forward rendering does not reuse previous textures like raytracer so 
        //overdrawing on the same output texture for UI does not matter 
        else texOutput= &ExecuteForwardRendering();

        //NOTE: UI PASS DOES mutate the input texture with the UI drawn on top of it
        if (DRAW_UI) ExecuteUIPass(*texOutput);
        //The texture that is then used for post process is NOT mutated by PP because it is only used for sampling
        //and outputitting to the default framebuffer which renders to screen
        if (DO_POST_PROCESS) ExecutePostProcessPass(*texOutput);
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
        m_isRenderStalled = false;

        m_framesSinceStart++;
        m_unmovingFrames++;
        
        if (!DO_STATIC_GEOMETRY)
        {
            m_uniformData.m_LightBlock.m_PointLightsCount = 0;
            m_frameGeometryMetrics = {};

            m_instanceMeshes.clear();
            m_emissiveInstanceIndices.clear();

            m_geometryUnit.ClearAll();
            ResetRuntimeMaterialId();
        }
        m_uiUnit.ClearAll();
        
        UpdatePassRenderState(RenderPassType::None);
    }

    std::string Renderer::ToStringBVH() const
    {
        return "TO STRING BVH: \n" + m_tlasTreeAligned.ToString(BVHToStringType::NodeBounds, nullptr,
            //TLAS leaf node to string function 
            [this](const BVHNodeStd430& tlasLeafNode) -> std::string
            {
                const Instance& instance = m_geometryUnit.m_CpuInstances[tlasLeafNode.m_ObjectStartIndex];
                const InstanceMesh& mesh = m_instanceMeshes[instance.m_MeshIndex];
                const ArrayInterval interval = mesh.m_BLASTreesInterval;
                const Vec3 aabbSize = tlasLeafNode.GetAABB().GetSize();
                if (::Math::ApproximateEqualsF(aabbSize.m_X, 0) || ::Math::ApproximateEqualsF(aabbSize.m_Y, 0) || ::Math::ApproximateEqualsF(aabbSize.m_Z, 0))
                    LogWarning(std::format("[BVH]: Found INVALID 0-value TLAS tree node: {}", tlasLeafNode.ToString()));

                //Here we add a red prefix if the split between the leaf node of the tlas tree and the transformed 
                //world root node bounds of the blas tree is wrong (the tlas leaf node bounds > blas root node bounds)
                std::string invalidBoundsPrefix = "";
                std::string invalidBoundsSuffix = "";
                const BVHNodeStd430& blasRootNode = m_blasTreesAligned[interval.m_StartIndex];
                AABB3D rootNodeWorldBounds = Math::ApplyMatrixToAABB(blasRootNode.GetAABB(), instance.m_ModelMatrix);
                AABB3D parentBounds = tlasLeafNode.GetAABB();
                if (parentBounds.GetSize().AnyAxisLessThan(rootNodeWorldBounds.GetSize()) || 
                    parentBounds.m_MinPos.AnyAxisGreaterThan(rootNodeWorldBounds.m_MinPos) ||
                    parentBounds.m_MaxPos.AnyAxisLessThan(rootNodeWorldBounds.m_MaxPos))
                {
                    invalidBoundsPrefix = ANSI_COLOR_RED;
                    invalidBoundsSuffix = ANSI_COLOR_CLEAR;
                }

                //LogWarning(std::format("Interval is: {} mesh index: {} instance index:{}", interval.m_Size, m_instances[node.m_ObjectStartIndex].m_MeshIndex));
                const IndexTriangle* trianglePtr = reinterpret_cast<const IndexTriangle*>(m_geometryUnit.m_CpuIndices[0]);
                return invalidBoundsPrefix + ToStringBVHNodes<IndexTriangle, STD_430_ALIGN>(&m_blasTreesAligned[0], interval.m_StartIndex,
                    interval.m_Size, trianglePtr, nullptr, BVHToStringType::NodeBounds,
                    [instance, &tlasLeafNode](const BVHNodeStd430& blasNode, const BVHNodeStd430* parentNode) -> std::string
                    {
                        const Vec3 aabbSize = blasNode.GetAABB().GetSize();
                        if (::Math::ApproximateEqualsF(aabbSize.m_X, 0) || ::Math::ApproximateEqualsF(aabbSize.m_Y, 0)
                            || ::Math::ApproximateEqualsF(aabbSize.m_Z, 0))
                        {
                            LogWarning(std::format("[BVH]: Found INVALID 0-value bounds for BLAS tree node: {}", blasNode.ToString()));
                        }

                        return std::format("[BLASNode Bounds:{}]",
                            Math::ApplyMatrixToAABB(blasNode.GetAABB(), instance.m_ModelMatrix).ToString());
                    },
                    //BLAS Leaf node to string function -> get vertices
                    [this, &instance, &mesh, &blasRootNode](const BVHNodeStd430& blasLeafNode) -> std::string
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

                            if (!Math::IsWithinBounds(blasLeafNode.GetAABB(), v0) || !Math::IsWithinBounds(blasLeafNode.GetAABB(), v1) ||
                                !Math::IsWithinBounds(blasLeafNode.GetAABB(), v2))
                            {
                                //LogError(std::format("[BVH]"));
                                verticesStr += ANSI_COLOR_RED + triangleStr + ANSI_COLOR_CLEAR;
                            }
                            if (Math::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v0) || Math::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v1) ||
                                Math::IsFullyOutsideBounds(blasLeafNode.GetAABB(), v2))
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
                const IndexTriangle* trianglePtr = reinterpret_cast<const IndexTriangle*>(&m_geometryUnit.m_CpuIndices[0]);
                std::string blasTreeString = ToStringBVHNodes<IndexTriangle>(&m_blasTreesAligned[0], interval.m_StartIndex,
                    interval.m_Size, trianglePtr, nullptr, BVHToStringType::NodeBounds);

                std::vector<Vec3> vertexPositions = {};
                const InstanceMesh& mesh = m_instanceMeshes[instance.m_MeshIndex];
                for (size_t j = 0; j < mesh.m_NumIndices; j++)
                {
                    vertexPositions.emplace_back(m_geometryUnit.m_CpuVertices[m_geometryUnit.m_CpuIndices[mesh.m_IndexOffset + j]].m_LocalPos);
                }

                result += std::format("\n[Instance]: Material(Idx:{}):{} Model:{} Vertices:{} \nMesh BLAS TREE(MeshIndex:{} IntervalStart:{} IntervalSize:{}):{}", 
                    instance.m_MaterialIndex, m_materialData[instance.m_MaterialIndex].ToString(), instance.m_ModelMatrix.ToString(),
                    ::Utils::ToStringIterable(vertexPositions), instance.m_MeshIndex, interval.m_StartIndex, interval.m_Size, blasTreeString);
            }
        }
        return result;
    }
    std::string Renderer::ToStringMetrics() const
    {
        std::string result = std::format("Metrics:\n Render calls:{}", 
            ::Utils::ToStringIterable(m_frameGeometryMetrics.m_RenderCallInvocations));
        //TODO: count how many sphere counts, etc
        return result;
    }
    std::string Renderer::ToStringAll() const
    {
        return std::format("DUMPING RENDERER DATA:\nCameraState:{}\nGEOMETRY DATA:\nVertex({}):{}\nIndex({}):{}\nInstances({}):{}\nBatches:{}", 
            m_engineState->m_CameraController->GetActiveCamera().ToString(),
            m_geometryUnit.m_CpuVertices.size(), ::Utils::ToStringIterable(m_geometryUnit.m_CpuVertices),
            m_geometryUnit.m_CpuIndices.size(), ::Utils::ToStringIterable(m_geometryUnit.m_CpuIndices),
            m_geometryUnit.m_CpuInstances.size(), ::Utils::ToStringIterable(m_geometryUnit.m_CpuInstances),
            m_geometryUnit.ToStringBatches());
    }
}