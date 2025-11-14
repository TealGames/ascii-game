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
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Math/PlatformMath.hpp"
#include "Core/Window/WindowManager.hpp"
#include "Core/Time/TimeKeeper.hpp"

#include "Utils/Data/ColorConstants.hpp"

namespace Rendering
{
    constexpr bool DO_RAYTRACING = true;
    constexpr std::uint32_t MAX_RAYTRACE_BOUNCES = 5;

    constexpr bool DO_LIGHTING = true;
    
    constexpr bool DRAW_LIGHT_AREAS = true;
    constexpr bool USE_LIGHT_COLOR_FOR_RANGE = true;
    constexpr Color LIGHT_AREA_COLOR = {255, 255, 255, 255};
    
    constexpr bool DO_SHADOWS = true;
    constexpr Vec2Int SHADOW_MAP_SIZE = {256, 256};
    constexpr Vec2Int SKYBOX_MAP_SIZE = {512, 512};
    constexpr float SHADOW_NEAR_DISTANCE = 0.001;
    constexpr float SHADOW_FAR_DISTANCE = 1000;

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
    constexpr float EXPOSURE = 0.6;

    constexpr size_t NO_RENDER_FRAME_COUNT_LIMIT = 0;
    constexpr size_t RENDER_FRAMES_COUNT = NO_RENDER_FRAME_COUNT_LIMIT;
    constexpr LogType STALL_LOG_TYPE = LogType::Warning;

    constexpr size_t INSTANCE_MAX_COUNT = 16;
    constexpr size_t INDEX_MAX_COUNT = 20000;
    constexpr size_t VERTEX_MAX_COUNT = 10000;
    constexpr size_t MATERIAL_MAX_COUNT = 5;
    constexpr size_t TEXTURE_MAX_COUNT = 5;
    constexpr size_t CIRCLE_SIDE_COUNT = 12;

    static const char* CORE_SHADER_NAMES[CORE_SHADER_COUNT] = { 
        "default", "forward_render", "shadow", "texture", 
        "post_process", "gaussian_blur", "ray_tracer", "skybox_converter"};

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

    //For compute shaders
    constexpr const char* TEXTURES_UNIFORM_NAME = "uTextures";
    constexpr const char* SKYBOX_UNIFORM_NAME = "uSkybox";
    constexpr const char* HORIZONTAL_FLAG_UNIFORM_NAME = "uIsHorizontal";
    constexpr const char* INPUT_TEXTURE_UNIFORM_NAME = "uTextureInput";
    constexpr const char* OUTPUT_TEXTURE_UNIFORM_NAME = "uTextureOutput";
    constexpr const char* BLUR_WEIGHTS_UNIFORM_NAME = "uWeights";

    constexpr const char* RAY_TRACING_MAX_RAY_BOUNCES_UNIFORM_NAME = "uMaxBounces";
    constexpr const char* UNMOVING_FRAME_NUMBER_UNIFORM_NAME = "uUnmovingFrameCount";
    constexpr const char* EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME = "uEmissiveMaterialCount";
    constexpr const char* INSTANCE_COUNT_UNIFORM_NAME = "uInstanceCount";

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

    std::string RenderBatch::ToString() const
    {
        return std::format("[Batch Shader:{} Texture:{} VertexStart:{} VertexCount:{} "
            "IndexStart:{} IndexCount:{} InstancesStart:{} InstancesCount:{}]", 
            m_Shader!=nullptr, m_Texture!=nullptr, m_VertexStartIndex, m_VertexCount, 
            m_IndicesStartIndex, m_IndicesCount, m_InstanceStartIndex, m_InstanceCount);
    }

    bool RenderPassData::UsesDefaultFrameBuffer() const
    {
        return m_FrameBuffer != nullptr;
    }

    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants
    Renderer::Renderer(const EngineState& engineState)
        : m_isInit(false), m_engineState(&engineState), m_uniformData(), //m_staticRenderData(),
        m_batches(), m_hashToBatchIndex(), m_graphicsManager(nullptr), m_frameGeometryMetrics(), m_runtimeMaterialId(),
        m_textureController(Backend::CreateTextureController()),
        m_imageController(Backend::CreateImageController()),
        m_vertices(), m_vertexIndices(), m_instances(), m_instanceMeshes(), m_emissiveInstanceIndices(),
        m_vertexBuffer(), m_indexBuffer(), m_instancedBuffer(), m_vertexLayout(), m_bufferController(&m_vertexLayout),
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
        m_emissiveInstanceIndexStorageBuffer(Backend::CreateShaderStorageBuffer(LIGHT_INDICES_SSBO_BLOCK_NAME))
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
            else if (passType == RenderPassType::Geometry && DO_HDR)
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
        m_vertexLayout = Backend::CreateVertexLayout();
        m_vertexBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(VertexType), VERTEX_MAX_COUNT, VertexAttributeAdvance::Vertex);
        m_indexBuffer = Backend::CreateIndexBuffer(nullptr, INDEX_MAX_COUNT);
        m_instancedBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(Instance), INSTANCE_MAX_COUNT, VertexAttributeAdvance::Instance);

        m_vertices.reserve(VERTEX_MAX_COUNT);
        m_vertexIndices.reserve(INDEX_MAX_COUNT);
        m_instances.reserve(INSTANCE_MAX_COUNT);
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

        m_bufferController.AddShaderBuffer(&m_vertexStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_indexStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_instanceStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_instanceMeshStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_materialStorageBuffer);
        m_bufferController.AddShaderBuffer(&m_emissiveInstanceIndexStorageBuffer);

        m_frameBuffer = Backend::CreateFrameBuffer();
        //TODO: right now all shadows have same resolution -> this might be a light component setting
        if (DO_SHADOWS)
        {
            m_engineState->m_GraphicsContext.m_GraphicsManager->AddShaderGlobalDefine("DO_SHADOWS");

            for (auto& map : m_shadowMaps) 
                map = CreateTextureCube(SHADOW_MAP_SIZE, TexelStorageType::Depth24);
        }
        /*m_skybox = CreateTextureCube(SKYBOX_MAP_SIZE, TexelStorageType::RGBA16F,
            { WrapBehavior::ClampEdge, WrapBehavior::ClampEdge, WrapBehavior::ClampEdge });*/

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
        
        const VertexLayoutBindIndex vertexBindIndex = m_bufferController.AddVertexBuffer(&m_vertexBuffer, &m_indexBuffer);
        std::vector<VertexAttribute> vertexAttributes = 
        { 
            VertexAttribute(0, 3, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_LocalPos)), 
            VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_UVPos)),
            VertexAttribute(2, 3, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_Normal)),
        };
        m_vertexLayout.AddAttributes(vertexBindIndex, vertexAttributes);

        const VertexLayoutBindIndex instancedBindIndex = m_bufferController.AddVertexBuffer(&m_instancedBuffer, nullptr);
        std::vector<VertexAttribute> instancedAttributes = 
        {
            VertexAttribute(3, 1, VertexAttributeBaseType::UnsignedInteger, false, offsetof(Instance, m_MaterialIndex)),
        };
        m_vertexLayout.AddAttributes(instancedBindIndex, instancedAttributes);
        m_vertexLayout.AddMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 4, false, sizeof(Vec4), offsetof(Instance, m_ModelMatrix));
        m_vertexLayout.AddMatrixAttribute(Vec2Int(3, 3), instancedBindIndex, 8, false, sizeof(Vec3), offsetof(Instance, m_NormalModelMatrix));

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
    RenderBatch* Renderer::TryGetBatch(const Shader& shader, const Texture& texture, std::uint32_t vertexCount)
    {
        const size_t hash= CalculateBatchHash(shader, texture, vertexCount);
        auto it = m_hashToBatchIndex.find(hash);

        if (it == m_hashToBatchIndex.end())
            return nullptr;
        return &(m_batches[it->second]);
    }
    RenderBatch* Renderer::TryGetSameDrawBatch(const Shader& shader, const Material& material, std::uint32_t vertexCount)
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
        return TryGetBatch(shader, *material.m_Albedo, vertexCount);
    }
    void Renderer::AddCompleteInstanceToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material)
    {
        AddInstanceDataToBatch(batch, modelMatrix, material);
        AddMeshInstanceToBatch(batch);
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

    size_t Renderer::CalculateBatchHash(const Shader& shader, const Texture&  texture, std::uint32_t totalVertices) const
    {
        const BatchKey batchKey = BatchKey(shader.GetId(), texture.GetInfo().m_id, totalVertices);
        return std::hash<BatchHash>{}(*reinterpret_cast<const BatchHash*>(&batchKey));
    }
    size_t Renderer::CalculateBatchHash(const RenderBatch& batch) const
    {
        return CalculateBatchHash(*batch.m_Shader, *batch.m_Texture, batch.m_VertexCount);
    }
    RenderBatch& Renderer::CreateBatch(Shader& shader, Material& material,
        const Vertex* vertexArray, const size_t vertexSize, const IndexType* indexArray, const size_t indicesSize, 
        const Mat4& modelMatrix, const bool isFinished)
    {
        SetMaterialAlbedoIfNull(material);

        RenderBatch& batch = m_batches.emplace_back(&shader, material.m_Albedo);
        if (vertexSize > 0 && vertexArray != nullptr)
        {
            AddVerticesToBatch(batch, vertexArray, vertexSize);
        }
        if (indicesSize > 0 && indexArray != nullptr)
        {
            AddIndicesToBatch(batch, indexArray, indicesSize);
        }
        
        AddInstanceDataToBatch(batch, modelMatrix, material);
        if (isFinished) FinishBatch(batch);
        return batch;
    }
    void Renderer::FinishBatch(RenderBatch& batch)
    {
        //NOTE: we can only add mesh instance once we finish a batch with all indices so we know what offset/size to use
        AddMeshInstanceToBatch(batch);
        m_hashToBatchIndex.emplace(CalculateBatchHash(batch), m_batches.size() - 1);
    }
    void Renderer::AddVertexToBatch(RenderBatch& batch, const Vertex& vertex)
    {
        //batch.m_Vertices.emplace_back(vertex);
        
        m_vertices.emplace_back(vertex);
        if (batch.m_VertexCount == 0)
            batch.m_VertexStartIndex = m_vertices.size() - 1;

        batch.m_VertexCount++;
        m_frameGeometryMetrics.m_TotalVertices++;
    }
    void Renderer::AddVerticesToBatch(RenderBatch& batch, const Vertex* vertexArray, const size_t vertexSize)
    {
        //NOTE: we do this before the insertion since start index is index greater than current last index
        if (batch.m_VertexCount == 0)
            batch.m_VertexStartIndex = m_vertices.size();

        m_vertices.insert(m_vertices.end(), vertexArray, vertexArray + vertexSize);
        
        batch.m_VertexCount += vertexSize;
        m_frameGeometryMetrics.m_TotalVertices += vertexSize;
    }
    void Renderer::AddIndicesToBatch(RenderBatch& batch, const std::array<IndexType, 3>& arr)
    {
        if (batch.m_IndicesCount == 0)
            batch.m_IndicesStartIndex = m_vertexIndices.size();

        //m_vertexIndices.insert(m_vertexIndices.end(), arr.begin(), arr.end());
        for (int i = 0; i < arr.size(); i++)
        {
            m_vertexIndices.push_back(batch.m_VertexStartIndex + arr[i]);
        }
        
        
        batch.m_IndicesCount += arr.size();
        m_frameGeometryMetrics.m_TotalIndices += arr.size();
    }
    void Renderer::AddIndicesToBatch(RenderBatch& batch, const IndexType* indexArray, const size_t indicesSize)
    {
        if (batch.m_IndicesCount == 0)
            batch.m_IndicesStartIndex = m_vertexIndices.size();

        //m_vertexIndices.insert(m_vertexIndices.end(), indexArray, indexArray + indicesSize);
        for (int i = 0; i < indicesSize; i++)
        {
            m_vertexIndices.push_back(batch.m_VertexStartIndex + indexArray[i]);
        }
        
        batch.m_IndicesCount += indicesSize;
        m_frameGeometryMetrics.m_TotalIndices += indicesSize;
    }
    void Renderer::AddInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Material& material)
    {
        Mat3 normalMatrix = modelMatrix.GetSlice<3, 3>();
        //Mat3 inversed = {};
        if (!normalMatrix.Inverse())
        {
            LogError(std::format("Attempted to add instance data to batch with model matrix:{} "
                "but 3x3 normal model matrix fialed to inverse:{}", modelMatrix.ToString(), normalMatrix.ToString()));
            return;
        }

        auto cachedMaterialIt = m_cachedMaterials.find(material.m_Name);
        if (cachedMaterialIt == m_cachedMaterials.end())
        {
            m_materialData.emplace_back(MaterialData(material, GetEnqueuedTextureIndex(material.m_Albedo)));
            cachedMaterialIt = m_cachedMaterials.emplace(material.m_Name, m_materialData.size() - 1).first;
        }
        /*LogError(std::format("Original:{} inversed:{} normaModel:{} inserted", modelMatrix.ToString(),
            normalMatrix.ToString(), normalMatrix.Transpose().ToString()));*/

        m_instances.emplace_back(cachedMaterialIt->second, modelMatrix, normalMatrix.Transpose());
        if (DO_RAYTRACING)
        {
            if (material.GetEmissiveColor().HasVisibleNonzeroRGB()) 
                m_emissiveInstanceIndices.emplace_back(m_instances.size() - 1);
            //LogWarning(std::format("Added instance meshes: {}", Utils::ToStringIterable<std::vector<InstanceMesh>, InstanceMesh>(m_instanceMeshes)));
            //if (m_instanceMeshes.size() >= PRE_ALLOCATED_INSTANCES_COUNT) LogError("surpassed");
        }

        if (batch.m_InstanceCount == 0)
            batch.m_InstanceStartIndex = m_instances.size() - 1;
        
        batch.m_InstanceCount++;
        m_frameGeometryMetrics.m_TotalInstances++;
        //Note: every time we add new instance data to the batch, we increase the index offset since we know
        //the current model has finished
    }
    void Renderer::AddMeshInstanceToBatch(RenderBatch& batch)
    {
        if (DO_RAYTRACING) m_instanceMeshes.emplace_back(InstanceMesh(batch.m_IndicesStartIndex, batch.m_IndicesCount));
    }
    int Renderer::GetEnqueuedTextureIndex(Texture* texture)
    {
        if (texture == nullptr)
            return INVALID_TEXTURE_INDEX;

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

    void Renderer::AddCallBox3DMulti(Shader& shader, Material& material, const Vec3& size, const Mat4& modelMatrix)
    {
        /*
        if (texture == nullptr)
        {
            constexpr size_t VERTEX_COUNT = 8;
            constexpr size_t INDEX_COUNT = 36;

            if (material->m_BaseColor.m_A == MAX_INT_COLOR_CHANNEL)
            {
                RenderBatch* sameStatebatch = TryGetBatch(shader, texture, INDEX_COUNT);
                if (sameStatebatch != nullptr)
                {
                    AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                    return;
                }
            }

            const WorldPosition3D halfSize = size / 2;
            //NOTE: with 8 vertex cube it is impossible to calculate normals since one vertex connects 3 sides
            Vertex vertices[VERTEX_COUNT] = { Vertex(halfSize, UV()),                       Vertex(halfSize * Vec3(1, -1, 1), UV()),
                                              Vertex(halfSize * Vec3(-1, -1, 1), UV()),     Vertex(halfSize * Vec3(-1, 1, 1), UV()),
                                              Vertex(halfSize * Vec3(1, 1, -1), UV()),      Vertex(halfSize * Vec3(1, -1, -1), UV()),
                                              Vertex(halfSize * Vec3(-1, -1, -1), UV()),    Vertex(halfSize * Vec3(-1, 1, -1), UV()) };

            //Front face, back face, right, left, top, bottom
            IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2,
                                               4, 5, 6, 4, 7, 6,
                                               4, 5, 1, 4, 0, 1,
                                               7, 6, 2, 7, 3, 2,
                                               4, 0, 3, 4, 7, 3,
                                               5, 1, 2, 5, 6, 2 };
            CreateBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, color, true);
            return;
        }
        */

        constexpr size_t VERTEX_COUNT = 24;
        constexpr size_t INDEX_COUNT = 36;
        //NOTE: since opaque objects can get depth tested, we can cram as many of them as we want into
        //a batch as long as they have the same state, but for transparent objects
        //they need to have their own batch to ensure correct draw order
        RenderBatch* sameStatebatch = TryGetSameDrawBatch(shader, material, INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }

        const WorldPosition3D halfSize = size / 2;
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
            halfSize* Vec3(1, -1, 1),
            halfSize* Vec3(-1, -1, 1),
            halfSize* Vec3(-1, 1, 1),
            //BACK FACE
            halfSize* Vec3(1, 1, -1),
            halfSize* Vec3(1, -1, -1),
            halfSize* Vec3(-1, -1, -1),
            halfSize* Vec3(-1, 1, -1)
        };

        //The size is in x, y, z axis 
        const Vec2 textureSize = material.m_Albedo->GetInfo().m_texelSize.AsFloat();
        //The size in texture pixel coords based on its world size
        const Vec3Int pixelSize = CalculateFaceSizeForTexture(size, material.m_Albedo->GetInfo().m_texelSize);

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
        uvs[9] = uvs[8] + Vec2(topBottomFaceSize.m_X,0);
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

        CreateBatch(shader, material, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, true);
    }

    void Renderer::AddCallSphere3DMulti(Shader& shader, Material& material, const float radius,
        const Mat4& modelMatrix)
    {
        //Note: this is the default UV method with longitudinal/"slices" (vertical) and latitudinal/"stacks" (horizontal) lines
        constexpr size_t HORIZONTAL_LINE_COUNT = 8;

        //Best shape is formed with 1.5 factor 
        //Note: the total number of vertices is horizontal-1 * vertical * 6 since we create square
        //for every 2 pairs going downward, thus needing to exlude the final horizontal row
        // + vertical * 3 (north pole)+ vertical*3 (south pole) since each vertical connects with top
        constexpr size_t VERTICAL_LINE_COUNT = HORIZONTAL_LINE_COUNT * 1.5f;
        constexpr size_t TOTAL_VERTEX_COUNT = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT + 2;
        constexpr size_t TOTAL_INDEX_COUNT = (HORIZONTAL_LINE_COUNT - 1) * VERTICAL_LINE_COUNT * 6 + VERTICAL_LINE_COUNT * 6;

        IndexType indices[TOTAL_INDEX_COUNT] = {};
        size_t currIndex = 0;
        
        RenderBatch* sameStatebatch = TryGetSameDrawBatch(shader, material, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddCompleteInstanceToBatch(*sameStatebatch, modelMatrix, material);
            return;
        }

        RenderBatch& batch= CreateBatch(shader, material, nullptr, TOTAL_VERTEX_COUNT, nullptr, TOTAL_INDEX_COUNT, modelMatrix, false);
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
                AddVertexToBatch(batch, Vertex{ pos * radius, UV(u, v), pos.GetNormalized() });

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
                //Note: no modulus needed since 
                aIndex = hi * (VERTICAL_LINE_COUNT)+vi;
                bIndex = hi * (VERTICAL_LINE_COUNT)+nextV;
                cIndex = (hi + 1) * (VERTICAL_LINE_COUNT)+vi;
                dIndex = (hi + 1) * (VERTICAL_LINE_COUNT)+nextV;

                AddIndicesToBatch(batch, { aIndex, cIndex, bIndex });
                AddIndicesToBatch(batch, { bIndex, cIndex, dIndex });
            }
        }

        //First we build the north pole vertex and create the indices
        //AddVertexToBatch(Vertex{ Vec3(0.0f, radius, 0.0f), UV(1.0f, 1.0f), ENGINE_UP_DIR });
        AddVertexToBatch(batch, Vertex{ Vec3(0.0f, radius, 0.0f), UV(1.0f, 1.0f), ENGINE_UP_DIR });
        IndexType poleIndex = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = vi;
            bIndex = (aIndex + 1) % VERTICAL_LINE_COUNT;
            AddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        //Finally, we connect all the bottom latitude/row verticies to the south pole vertex
        //AddVertexToBatch(Vertex{ Vec3(0.0f, -1.5 * radius, 0.0f), UV(0.0f, 0.0f), -ENGINE_UP_DIR });
        AddVertexToBatch(batch, Vertex{ Vec3(0.0f, -1.5 * radius, 0.0f), UV(0.0f, 0.0f), -ENGINE_UP_DIR });
        poleIndex++;
        const IndexType bottomStartIndex = poleIndex - VERTICAL_LINE_COUNT - 1;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = bottomStartIndex + vi;
            if (vi < VERTICAL_LINE_COUNT - 1) bIndex = aIndex + 1;
            else bIndex = bottomStartIndex;
            AddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        FinishBatch(batch);
        //LogError(ToStringAll());
    }

    void Renderer::AddCallBox3D(Material* material, const Vec3& size, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseShader(), GetMaterialOrDefault(material), size, modelMatrix);
    }
    void Renderer::AddCallSphere3D(Material* material, const float radius, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseShader(), GetMaterialOrDefault(material), radius, modelMatrix);
    }

    void Renderer::AddCallTextureSphere3D(Material* material, const float radius, const Mat4& modelMatrix)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), radius, modelMatrix);
    }
    void Renderer::AddCallTextureBox3D(Material* material, const Vec3& size, const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseTextureShader(), GetMaterialOrDefault(material), size, modelMatrix);
    }
    void Renderer::AddCallPlane3D(Material* material, const Vec2& size, const Mat4& modelMatrix)
    {
        constexpr size_t TOTAL_INDEX_COUNT = 6;
        constexpr size_t TOTAL_VERTEX_COUNT = 4;

        Shader& baseShader = GetBaseTextureShader();
        Material& baseMaterial = GetMaterialOrDefault(material);
        RenderBatch* sameStatebatch = TryGetSameDrawBatch(baseShader, baseMaterial, TOTAL_INDEX_COUNT);
        if (sameStatebatch != nullptr)
        {
            AddCompleteInstanceToBatch(*sameStatebatch, modelMatrix, baseMaterial);
            return;
        }

        Vertex vertices[TOTAL_VERTEX_COUNT] = {
            Vertex(Vec3(-size.m_X/2, 0, -size.m_Y/2), UV(0, 0), ENGINE_UP_DIR), 
            Vertex(Vec3(size.m_X/2, 0, -size.m_Y/2), UV(1, 0), ENGINE_UP_DIR),
            Vertex(Vec3(size.m_X/2, 0, size.m_Y/2), UV(1, 1), ENGINE_UP_DIR),
            Vertex(Vec3(-size.m_X/2, 0, size.m_Y/2), UV(0, 1), ENGINE_UP_DIR)
        };
        IndexType indices[TOTAL_INDEX_COUNT] = {0, 1, 2, 2, 3, 0};
        CreateBatch(baseShader, baseMaterial, vertices, TOTAL_VERTEX_COUNT, indices, TOTAL_INDEX_COUNT, modelMatrix, true);
    }

    void Renderer::AddCallPointLight(const WorldPosition3D& worldPos, const Quat& worldRot, const float radius, const Color color)
    {
        if (m_uniformData.m_LightBlock.m_PointLightsCount >= MAX_POINT_LIGHTS)
        {
            LogError(std::format("Attempted to add point light call at:{} colored:{} "
                "but max points lights have been reached", worldPos.ToString(), color.ToString()));
            return;
        }

        auto& pointlightData = m_uniformData.m_LightBlock.m_PointLights[m_uniformData.m_LightBlock.m_PointLightsCount];
        pointlightData = PointLightData(worldPos, color, radius);
        pointlightData.m_ShadowMapIndex = m_uniformData.m_LightBlock.m_PointLightsCount;
        m_uniformData.m_ExtraPointLightData[m_uniformData.m_LightBlock.m_PointLightsCount] = ExtraPointLightData{worldRot};
        m_uniformData.m_LightBlock.m_PointLightsCount++;

        if (DRAW_LIGHT_AREAS)
        {
            Material lightMaterial = Material("[$LightArea:"+std::to_string(GenerateRuntimeMaterialId())+"]", &GetDefaultAlbedo(),
                USE_LIGHT_COLOR_FOR_RANGE ? color : LIGHT_AREA_COLOR);

            AddCallSphere3DMulti(GetCoreShader(CoreShader::Texture), lightMaterial, std::min(0.1f * radius, 1.0f), 
                CalculateModelMatrix(nullptr, worldPos, Vec3::One(), Quat::Identity()));
        }
    }
    void Renderer::AddCallDirectionalLight(const Vec3& dir, const Color color)
    {
        m_uniformData.m_LightBlock.m_DirLight = DirectionalLightData(dir, color);
    }

    void Renderer::AddCallModel(Model3d& model, const Mat4& modelMatrix)
    {
        ModelMesh* mesh = nullptr;
        //LogWarning(std::format("Found mesh group: {}", model.m_MeshGroups.size()));
        for (auto& meshGroup : model.m_MeshGroups)
        {
            //LogWarning(std::format("Found mesh group indices : {}", meshGroup.m_MeshIndices.size()));
            for (auto& meshIndex : meshGroup.m_MeshIndices)
            {
                mesh = &(model.m_Meshes[meshIndex]);

                //TODO: right now we only care about color from material but we should be able 
                // add all args to batch (maybe accept material?)
                CreateBatch(GetBaseTextureShader(), mesh->m_Material, &(mesh->m_Vertices[0]), mesh->m_Vertices.size(),
                    &(mesh->m_Indices[0]), mesh->m_Indices.size(), modelMatrix * meshGroup.m_GlobalTransform, true);
            }
        }
    }

    void Renderer::SetSkybox(Texture* texture)
    {
        m_skybox = texture;
        /*
        Shader& skyboxConverter = GetCoreShader(CoreShader::SkyboxConverted);
        BindShader(skyboxConverter);
        BindFrameBuffer(&m_frameBuffer);

        const Vec2Int faceTextureSize = m_skybox.GetData().m_texelSize;
        Backend::SetViewport(faceTextureSize.m_X, faceTextureSize.m_Y);

        SlotIndex skyboxSlot = m_textureController.TryBindToFreeSlot<Texture>(*texture);
        skyboxConverter.TrySetUniform(UniformDataType::Sampler2D, SKYBOX_UNIFORM_NAME, &skyboxSlot);

        std::array<Mat4, 6> viewMatrices = {};
        Mat4 projMatrix = {};
        CalculateCubeMapMatrices(Vec3(), 0.1, 10, viewMatrices, projMatrix);
        for (std::uint8_t i = 0; i < 6; i++)
        {
            SetViewerData(Vec3::Zero(), viewMatrices[i], projMatrix);
            m_frameBuffer.SetOutputTextureCube(FrameBufferAttachmentType::Color0, &m_skybox, static_cast<TextureCubeFace>(i));
            Backend::ClearBackground();

            //NOTE: since the vertices are stored in the shader, we can just invoke with
            //how many verticies to do
            Backend::DrawVertices(36);
        }

        UnbindActiveShader();
        m_textureController.TryRemoveFromSlot(skyboxSlot);
        //This forces the viewport to be set back to rendering for the window
        m_engineState->m_GraphicsContext.m_Window->ForceSizeUpdate();
        */
    }

    void Renderer::RenderStartActions() const
    {
        Backend::BeginRenderingMarker();
    }
    void Renderer::SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix)
    {
        m_viewerUniformBuffer.TryWriteData("worldPos", sizeof(Vec3), worldPos.GetMemPointer());
        if (!m_viewerUniformBuffer.TryWriteData("viewMatrix", sizeof(Mat4),
            viewMatrix.GetMemPointer()))
        {
            LogError(std::format("Attempted to write view matrix to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteData("projectionMatrix", sizeof(Mat4),
            projMatrix.GetMemPointer()))
        {
            LogError(std::format("Attempted to write projection matrix to viewer uniform buffer but failed"));
            return;
        }
    }
    void Renderer::SetViewerData(const WorldPosition3D& worldPos, const Mat4& viewMatrix, const Mat4& projMatrix,
        const Vec3& forwardDir, const Vec3& rightDir, const Vec3& upDir, const float yFov)
    {
        SetViewerData(worldPos, viewMatrix, projMatrix);
        if (!m_viewerUniformBuffer.TryWriteData("forwardDir", sizeof(Vec3), forwardDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write forwardDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteData("rightDir", sizeof(Vec3), rightDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write rightDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteData("upDir", sizeof(Vec3), upDir.GetMemPointer()))
        {
            LogError(std::format("Attempted to write upDir to viewer uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteData("yFov", sizeof(float), &yFov))
        {
            LogError(std::format("Attempted to write yFov to viewer uniform buffer but failed"));
            return;
        }
    }
    void Renderer::DrawBatch(RenderBatch& batch)
    {
        Backend::DrawUploadedIndexBufferInstanced(0,
            batch.m_IndicesStartIndex * m_indexBuffer.GetElementSize(),
            batch.m_IndicesCount, batch.m_InstanceStartIndex, batch.m_InstanceCount);
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

                for (size_t k = 0; k < m_batches.size(); k++)
                {
                    auto& batch = m_batches[k]; 
                    if (batch.m_InstanceCount == 0)
                        continue;

                    DrawBatch(batch);
                }
            }
        }
   
        UnbindActiveShader();
        //This forces the viewport to be set back to rendering for the window
        m_engineState->m_GraphicsContext.m_Window->ForceSizeUpdate();
    }
    void Renderer::ExecuteLightingAndGeometryPass(const SlotIndex* shadowCubeMapSlots)
    {
        UpdatePassRenderState(RenderPassType::Geometry);
        if (DO_HDR)
        {
            m_boundFrameBuffer->SetOutputRenderBuffer(FrameBufferAttachmentType::Depth, &m_hdrDepthRenderBuffer);
            m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color0, &m_hdrColorOutput);
            m_boundFrameBuffer->SetOutputTexture(FrameBufferAttachmentType::Color1, &m_brightnessOutput);
        }
        Backend::ClearBackground();

        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        //TODO: this is still a problem since multiple flushes per frame means multiple updates
        const bool needsViewMatrixUpdate = Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::ViewMatrix);
        const bool needsProjMatrixUpdate = Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::PlatformProjMatrix);

        SetViewerData(camera.GetTransform().GetGlobalPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix);
        //TODO: optimize so we check if light block data changed from last render batch and only then write buffer
        m_lightUniformBuffer.WriteData(0, sizeof(LightBlockData), &m_uniformData.m_LightBlock);

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


        for (int i = 0; i < m_batches.size(); i++)
        {
#ifdef GRAPHICS_VERBOSE_LOG
            LogWarning(std::format("Flushing batch:{}/{}", i + 1, m_batches.size()));
#endif
            auto& batch = m_batches[i];

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
                if (slot == INVALID_SLOT_INDEX)

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
            DrawBatch(batch);
        }

        if (lastBatchShader != nullptr) unbindLastBatchShader();
        if (lastBatchTexture != nullptr) removeLastBatchTexture();

        //LogWarning(std::format("LIGHT PASS Texture controler after pass: {}", m_textureController.ToString()));
        //LogError(std::format("HDR color texture: {}", Utils::ToStringMemory(writePtr, byteSize)));
    } 
    void Renderer::ExecuteForwardRendering()
    {
        //NOTE: we do this to ensure that we only add any data as long as all 3 buffers have enough space
        const auto& freeVertexSeg = m_vertexBuffer.TryGetFreeSegment(m_frameGeometryMetrics.m_TotalVertices);
        const auto& freeIndexSeg = m_indexBuffer.TryGetFreeSegment(m_frameGeometryMetrics.m_TotalIndices);
        const auto& freeInstanceSeg = m_instancedBuffer.TryGetFreeSegment(m_frameGeometryMetrics.m_TotalInstances);
        if (freeVertexSeg == std::nullopt || freeIndexSeg == std::nullopt || freeInstanceSeg == std::nullopt)
        {
            m_isRenderStalled = true;
            const std::string message = std::format("Stalling vertex:{} index:{} instance:{}",
                freeVertexSeg == std::nullopt, freeIndexSeg == std::nullopt, freeIndexSeg == std::nullopt);
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }

        FencedBufferSegment& vertexFenceSeg = m_vertexBuffer.WriteDataFenced(&m_vertices[0], freeVertexSeg.value());
        FencedBufferSegment& indexFenceSeg = m_indexBuffer.WriteDataFenced(&m_vertexIndices[0], freeIndexSeg.value());
        FencedBufferSegment& instanceFenceSeg = m_instancedBuffer.WriteDataFenced(&m_instances[0], freeInstanceSeg.value());

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

        ExecuteLightingAndGeometryPass(DO_SHADOWS ? &shadowCubeMapSlots[0] : nullptr);
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
        Backend::SetDepthStatus(false);
        Backend::SetSrgbConversionStatus(true);
        
        //We just draw 3 vertices -> note we do not need vertex buffer since we use vertices defined in vertex shader
        Backend::DrawVertices(3);

        //NOTE: always unbind shader before switching shader settings like depth or srgb conversion status
        UnbindActiveShader();
        Backend::SetDepthStatus(true);
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

      
         //TODO: get propert emissive material count
        const std::uint32_t emissiveMaterialCount = m_emissiveInstanceIndices.size();
        rayTraceShader.TrySetUniform(UniformDataType::Uint, EMISSIVE_MATERIAL_COUNT_UNIFORM_NAME, &emissiveMaterialCount);
        const std::uint32_t instanceCount = m_instances.size();
        if (instanceCount == 0)
        {
            LogError(std::format("Attempted to execute ray tracing but there are no instances"));
            return;
        }
        rayTraceShader.TrySetUniform(UniformDataType::Uint, INSTANCE_COUNT_UNIFORM_NAME, &instanceCount);

        const SlotIndex inputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(m_hdrColorOutput, AccessPermissions::ReadWrite);
        const SlotIndex outputTextureSlot = m_imageController.TryBindToFreeSlot<Texture>(m_ioTexture, AccessPermissions::Write);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, INPUT_TEXTURE_UNIFORM_NAME, &inputTextureSlot);
        rayTraceShader.TrySetUniform(UniformDataType::Image2D, OUTPUT_TEXTURE_UNIFORM_NAME, &outputTextureSlot);


        m_vertexStorageBuffer.WriteData(0, m_vertices.size() * m_vertexBuffer.GetElementSize(), &m_vertices[0]);
        m_indexStorageBuffer.WriteData(0, m_vertexIndices.size() * m_indexBuffer.GetElementSize(), &m_vertexIndices[0]);
        m_instanceStorageBuffer.WriteData(0, m_instances.size() * m_instancedBuffer.GetElementSize(), &m_instances[0]);
        /*LogWarning(std::format("Instance mesh buffer elements:{} write:{} bytes:{}", m_instanceMeshStorageBuffer.GetAllocatedByteSize() / sizeof(InstanceMesh), 
            m_instanceMeshes.size(), m_instanceMeshes.size() * sizeof(InstanceMesh)));*/
        m_instanceMeshStorageBuffer.WriteData(0, m_instanceMeshes.size() * sizeof(InstanceMesh), &m_instanceMeshes[0]);
        if (emissiveMaterialCount > 0)
        {
            m_emissiveInstanceIndexStorageBuffer.WriteData(0, 
                m_emissiveInstanceIndices.size() * sizeof(std::uint32_t), &m_emissiveInstanceIndices[0]);
        }

        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        Vec3 worldFoward, worldUp, worldRight;
        camera.GetTransform().CalculateWorldDirections(&worldFoward, &worldUp, &worldRight);
        SetViewerData(camera.GetTransform().GetGlobalPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix, 
            worldFoward, worldRight, worldUp, camera.GetSettings().m_FieldOfViewYRadians);
        const Vec2Int windowSize = m_engineState->m_GraphicsContext.m_Window->GetSize();

        if (Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::ViewMatrix))
            m_unmovingFrames = 0;
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
                Utils::ToStringIterable<std::vector<VertexType>, VertexType>(m_vertices),
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
            
        //LogWarning(std::format("Frame number: {}", m_framesSinceStart));
        m_materialStorageBuffer.WriteData(0, m_materialData.size() * sizeof(MaterialData), &m_materialData[0]);
        /*for (const auto& batch : m_batches)
        {
            for (const auto& data : batch.m_InstanceData) LogWarning(data.ToString());
        }*/

        if (DO_RAYTRACING) ExecuteRayTracing();
        else ExecuteForwardRendering();

        //TODO: you should be able to do pp without hdr too
        ExecutePostProcessPass();
    }

    void Renderer::RenderBuffer()
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        //BLOOM_THRESHOLD = 2 * std::abs(std::sin(m_engineState->m_TimeKeeper->GetTimeSinceInit(TimeUnit::Seconds)));
        //LogWarning(std::format("BLoom is: {} now: {}", BLOOM_THRESHOLD, m_engineState->m_TimeKeeper->GetTimeSinceInit(TimeUnit::Milliseconds)));
        //LogWarning("RENDER");
        if (!m_batches.empty())
        {
            RenderStartActions();
            FlushBatches();

            //TODO: is this the best option for perofmrance amd should all be force cleared?
            //ideally we want to remove from middle, but that forces shifts in memory which may greatloy reduce performance
            //Tradeoff: performance cost for erasing some > performance cost of clearing all/having to reallocate frequent batches?
            m_batches.clear();
            m_hashToBatchIndex.clear();
        }
        RenderEndActions();
    }

    void Renderer::RenderEndActions()
    {
        Backend::EndRenderingMarker();

        m_uniformData.m_CameraUpdatedThisFrame = false;
        m_uniformData.m_LightBlock.m_PointLightsCount = 0;

        m_framesSinceStart++;
        m_unmovingFrames++;
        m_frameGeometryMetrics = {};

        m_vertices.clear();
        m_vertexIndices.clear();
        m_instances.clear();
        m_instanceMeshes.clear();
        m_emissiveInstanceIndices.clear();
        m_batches.clear();

        ResetRuntimeMaterialId();
        UpdatePassRenderState(RenderPassType::None);
    }

    std::string Renderer::ToStringBatches() const
    {
        std::string result = std::format("TOTAL({})\n ", m_batches.size());
        for (const auto& batch : m_batches)
        {
            result += "\nBatch:" + batch.ToString() + "\n";
        }
        return result;
    }
    std::string Renderer::ToStringAll()
    {
        return std::format("DUMPING RENDERER DATA:\nCameraState:{}\nVertex({}):{}\nIndex({}):{}\nInstances({}):{}\nBatches:{}", 
            m_engineState->m_CameraController->GetActiveCamera().ToString(),
            m_vertices.size(), Utils::ToStringIterable<std::vector<VertexType>, VertexType>(m_vertices),
            m_vertexIndices.size(), Utils::ToStringIterable<std::vector<IndexType>, IndexType>(m_vertexIndices),
            m_instances.size(), Utils::ToStringIterable<std::vector<InstanceType>, InstanceType>(m_instances),
            ToStringBatches());
    }
}