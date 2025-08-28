//NOT USED
#include "pch.hpp"
#include <optional>
#include <queue>
#include "StaticGlobals.hpp"

#include "Core/Rendering/GameRenderer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/EngineState.hpp"
#include "Core/Camera/CameraController.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/RenderingBackend.hpp"
#include "Math/PlatformMath.hpp"
#include "Core/Window/WindowManager.hpp"

#include "Utils/Data/ColorConstants.hpp"

namespace Rendering
{
    constexpr bool DO_LIGHTING = true;
    constexpr bool DO_SHADOWS = true;
    constexpr bool DRAW_LIGHT_AREAS = true;
    constexpr Utils::Color LIGHT_AREA_COLOR_FROM_LIGHT = Utils::Color(0, 0, 0, 0);
    constexpr Utils::Color LIGHT_AREA_COLOR = {255, 255, 255, 255};
    
    constexpr Vec2Int SHADOW_MAP_SIZE = {1024, 1024};
    constexpr float SHADOW_NEAR_DISTANCE = 0.001;
    constexpr float SHADOW_FAR_DISTANCE = 1000;

    constexpr size_t NO_RENDER_FRAME_COUNT_LIMIT = 0;
    constexpr size_t RENDER_FRAMES_COUNT = NO_RENDER_FRAME_COUNT_LIMIT;
    constexpr LogType STALL_LOG_TYPE = LogType::Warning;

    constexpr size_t PRE_ALLOCATED_SHAPES = 4;
    constexpr size_t PRE_ALLOCATED_INDICES_COUNT = 1000;
    constexpr size_t PRE_ALLOCATED_VERTICES_COUNT = 300;
    constexpr size_t CIRCLE_SIDE_COUNT = 12;

    static const char* CORE_SHADER_NAMES[CORE_SHADER_COUNT] = { "default", "forward_render", "shadow", "texture"};

    constexpr const char* VIEW_MATRIX_UNIFORM_NAME = "uViewMatrix";
    constexpr const char* PROJ_MATRIX_UNIFORM_NAME = "uProjectionMatrix";
    constexpr const char* TEXTURE_UNIFORM_NAME = "uAlbedo";
    constexpr const char* SHADOW_MAP_UNIFORM_NAME = "uShadowMaps";
    constexpr const char* SHADOW_TOGGLE_UNIFORM_NAME = "uDoShadows";

    std::string Vertex::ToString() const
    {
        return std::format("[{}]", m_Pos.ToString());
    }
    std::string InstanceData::ToString() const
    {
        return std::format("[Color:{} ModelMatrix:{}]", m_Color.ToString(), m_ModelMatrix.ToString());
    }

    PointLightData::PointLightData() : PointLightData({}, {}, 0) {}
    PointLightData::PointLightData(const WorldPosition3D& pos, const Vec4& color, const float radius)
        : m_Pos(pos), m_Color(color), m_Radius(radius), _padding0(0), _padding1{}, m_ShadowMapIndex(-1) {}

    DirectionalLightData::DirectionalLightData() : DirectionalLightData({}, {}) {}
    DirectionalLightData::DirectionalLightData(const Vec3& dir, const Vec4& color)
        : m_Direction(dir), m_Color(color), _padding0(0) {}

    std::string RenderBatch::ToString() const
    {
        return std::format("[Batch Shader:{} Texture:{} Vertices:{} Indices:{} Instances:{}]", 
            m_Shader!=nullptr, m_Texture!=nullptr,
            Utils::ToStringIterable<std::vector<VertexType>, VertexType>(m_Vertices), //m_IndexOffset,
            Utils::ToStringIterable<std::vector<IndexType>, IndexType>(m_VertexIndices),
            Utils::ToStringIterable<std::vector<InstanceType>, InstanceType>(m_InstanceData));
    }

    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants

    Renderer::Renderer(const EngineState& engineState)
        : m_isInit(false), m_engineState(&engineState), m_uniformData(), //m_staticRenderData(),
        m_renderCalls(), m_textData(), m_textureData(), m_batches(), m_hashToBatchIndex(), 
        m_layout(), m_bufferController(&m_layout), m_textureController(),
        m_vertexBuffer(), m_indexBuffer(), m_instancedBuffer(), m_viewerUniformBuffer(), m_lightUniformBuffer(),
        m_frameDrawCalls(0), m_isRenderStalled(false), m_framesSinceStart(0), 
        m_frameBuffer(), m_shadowMaps(), m_coreShaders({})
    {
        
    }

    void Renderer::Init()
    {
        //We reserve one for current batch, but also keep it as vector for future in case we do rendering in one go
        //m_batches.reserve(1);

        m_layout = Backend::CreateVertexLayout();
        m_vertexBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(VertexType), PRE_ALLOCATED_VERTICES_COUNT, VertexAttributeAdvance::Vertex);
        m_indexBuffer = Backend::CreateIndexBuffer(nullptr, PRE_ALLOCATED_INDICES_COUNT);
        m_instancedBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(InstanceData), PRE_ALLOCATED_SHAPES, VertexAttributeAdvance::Instance);
        m_viewerUniformBuffer = Backend::CreateUniformBuffer();
        m_lightUniformBuffer = Backend::CreateUniformBuffer();

        //TODO: right now all shadows have same resolution -> this might be a light component setting
        for (auto& map : m_shadowMaps) map = CreateTextureCube(SHADOW_MAP_SIZE, InternalStorage::Depth24);
        m_frameBuffer = Backend::CreateFrameBuffer();
        
        const VertexLayoutBindIndex vertexBindIndex = m_bufferController.AddVertexBuffer(&m_vertexBuffer, &m_indexBuffer);
        std::vector<VertexAttribute> vertexAttributes = 
        { 
            VertexAttribute(0, 3, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_Pos)), 
            VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_UVPos)),
            VertexAttribute(2, 3, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_Normal)),
        };
        m_bufferController.AddVertexBufferAttributes(vertexBindIndex, vertexAttributes);

        const VertexLayoutBindIndex instancedBindIndex = m_bufferController.AddVertexBuffer(&m_instancedBuffer, nullptr);
        std::vector<VertexAttribute> instancedAttributes = 
        {
            VertexAttribute(3, 4, VertexAttributeBaseType::Float, false, offsetof(InstanceData, m_Color)) 
        };
        m_bufferController.AddVertexBufferAttributes(instancedBindIndex, instancedAttributes);
        m_bufferController.AddVertexBufferMatrixAttribute(Vec2Int(4, 4), instancedBindIndex, 4, false, sizeof(Vec4), offsetof(InstanceData, m_ModelMatrix));
        m_bufferController.AddVertexBufferMatrixAttribute(Vec2Int(3, 3), instancedBindIndex, 8, false, sizeof(Vec3), offsetof(InstanceData, m_NormalModelMatrix));

        m_isInit = true;
    }
    bool Renderer::WasInit() const
    {
        return m_isInit;
    }

    void Renderer::InitCoreShaders() const
    {
        for (size_t i = 0; i < CORE_SHADER_COUNT; i++)
        {
            m_coreShaders[i] = m_engineState->m_GraphicsContext.m_GraphicsManager->TryGetShaderMutable(CORE_SHADER_NAMES[i]);
            if (m_coreShaders[i] == nullptr)
            {
                LogError(std::format("Core shader at index:{} could not be retrieved by name:{}", i, CORE_SHADER_NAMES[i]));
                return;
            }
        }
    }

    //Shader* Renderer::GetDefaultShader() const
    //{
    //    return m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultShaderMutable();
    //}
    //Shader* Renderer::GetTextureShader() const
    //{
    //    return m_engineState->m_GraphicsContext.m_GraphicsManager->GetTextureShaderMutable();
    //}
    //Shader* Renderer::GetForwardRenderShader() const
    //{
    //    return m_engineState->m_GraphicsContext.m_GraphicsManager->GetForwardRenderShaderMutable();
    //}

    Shader* Renderer::GetCoreShader(const CoreShader shader) const
    {
        if (m_coreShaders[0] == nullptr) InitCoreShaders();
        return m_coreShaders[static_cast<CoreShaderIntegralType>(shader)];
    }
    Shader* Renderer::GetBaseShader() const
    {
        if (DO_LIGHTING) return GetCoreShader(CoreShader::ForwardRender);
        return GetCoreShader(CoreShader::Default);
    }
    Shader* Renderer::GetBaseTextureShader() const
    {
        if (DO_LIGHTING) return GetCoreShader(CoreShader::ForwardRender);
        return GetCoreShader(CoreShader::Default);
    }
    Texture* Renderer::GetBaseAlbedo() const
    {
        return m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultAlbedoMutable();
    }
    Texture* Renderer::GetMaterialAlbedo(Material& material) const
    {
        if (material.m_Albedo == nullptr)
            return GetBaseAlbedo();
        return material.m_Albedo;
    }
    RenderBatch* Renderer::TryGetBatch(const Shader* shader, const Texture* texture, std::uint32_t vertexCount)
    {
        const size_t hash= CalculateBatchHash(shader, texture, vertexCount);
        auto it = m_hashToBatchIndex.find(hash);

        if (it == m_hashToBatchIndex.end())
            return nullptr;
        return &(m_batches[it->second]);
    }

    size_t Renderer::CalculateBatchHash(const Shader* shader, const Texture* texture, std::uint32_t totalVertices) const
    {
        const BatchKey batchKey = BatchKey(shader == nullptr ? INVALID_OBJ_ID : shader->GetId(),
            texture == nullptr ? INVALID_OBJ_ID : texture->GetData().m_id, totalVertices);
        return std::hash<BatchHash>{}(*reinterpret_cast<const BatchHash*>(&batchKey));
    }
    size_t Renderer::CalculateBatchHash(const RenderBatch& batch) const
    {
        return CalculateBatchHash(batch.m_Shader, batch.m_Texture, batch.m_VertexIndices.size());
    }
    RenderBatch& Renderer::CreateBatch(Shader* shader, Texture* texture,
        const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize, 
        const Mat4& modelMatrix, const Utils::Color& color, const bool isFinished)
    {
        RenderBatch& batch = m_batches.emplace_back(shader, texture);
        /*if (m_batches.size() == 2)
            LogError(std::format("reached batch size"));*/
        if (vertexSize > 0)
        {
            if (vertexArray != nullptr)
            {
                batch.m_Vertices.insert(m_batches.back().m_Vertices.begin(),
                    vertexArray, vertexArray + vertexSize);
            }
            else batch.m_Vertices.reserve(vertexSize);
        }
        if (indicesSize > 0)
        {
            if (indexArray != nullptr)
            {
                batch.m_VertexIndices.insert(m_batches.back().m_VertexIndices.end(),
                    indexArray, indexArray + indicesSize);
            }
            else batch.m_VertexIndices.reserve(indicesSize);
        }
        
        AddInstanceDataToBatch(batch, modelMatrix, color);
        if (isFinished) FinishBatch(batch);
        return batch;
    }
    void Renderer::FinishBatch(RenderBatch& batch)
    {
        m_hashToBatchIndex.emplace(CalculateBatchHash(batch), m_batches.size() - 1);
    }
    void Renderer::AddVertexToBatch(RenderBatch& batch, const Vertex& vertex)
    {
        batch.m_Vertices.emplace_back(vertex);
    }
    void Renderer::AddIndicesToBatch(RenderBatch& batch, const std::array<IndexType, 3>& arr)
    {
        batch.m_VertexIndices.insert(m_batches.back().m_VertexIndices.end(),
            arr.begin(), arr.end());
    }
    void Renderer::AddInstanceDataToBatch(RenderBatch& batch, const Mat4& modelMatrix, const Utils::Color& color)
    {
        Mat3 normalMatrix = modelMatrix.GetSlice<3, 3>();
        if (!normalMatrix.Inverse(&normalMatrix))
        {
            LogError(std::format("Attempted to add instance data to batch with model matrix:{} "
                "but 3x3 normal model matrix fialed to inverse:{}", modelMatrix.ToString(), normalMatrix.ToString()));
            return;
        }
        //LogError(std::format("Adding model:{} normal model:{}", modelMatrix.ToString(), normalMatrix.Transpose().ToString()));
        batch.m_InstanceData.emplace_back(color.GetNormalized(), modelMatrix, normalMatrix.Transpose());
        //Note: every time we add new instance data to the batch, we increase the index offset since we know
        //the current model has finished
    }

    Vec3Int Renderer::CalculateFaceSizeForTexture(const WorldPosition3D& worldSize, const Vec2Int textureSize)
    {
        return Vec3Int(worldSize.m_X / (2*worldSize.m_X + 2*worldSize.m_Z) * textureSize.m_X, 
                       worldSize.m_Y / (worldSize.m_Y + 2 * worldSize.m_Z) * textureSize.m_Y, 
                       worldSize.m_Z/ (2*worldSize.m_Z + 2*worldSize.m_X) * textureSize.m_X);
    }

    void Renderer::AddCallRectangle2DMulti(Shader* shader, Texture* texture, const Vec2& worldSize,
        const Mat4& modelMatrix, const Utils::Color& color)
    {
        constexpr size_t VERTEX_COUNT = 4;
        constexpr size_t INDEX_COUNT = 6;

        if (color.m_A == Utils::MAX_CHANNEL_VALUE)
        {
            RenderBatch* sameStatebatch = TryGetBatch(shader, texture, INDEX_COUNT);
            if (sameStatebatch != nullptr)
            {
                AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                return;
            }
        }

        const WorldPosition3D halfSize = Vec3(worldSize / 2, 0);
        //Start with top right vertex, then bottom right, then bottom left, top left
        const Vertex vertices[VERTEX_COUNT] = { Vertex(halfSize, UV(1, 1)), Vertex(halfSize * Vec3(1, -1, 0), UV(1, 0)),
                                     Vertex(halfSize * Vec3(-1, -1, 0), UV(0 ,0)), Vertex(halfSize * Vec3(-1, 1, 0), UV(0, 1)) };

        
        IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2 };
        CreateBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, color, true);
    }
    void Renderer::AddCallBox3DMulti(Shader* shader, Texture* texture, const Vec3& size,
        const Mat4& modelMatrix, const Utils::Color& color)
    {
        if (texture == nullptr)
        {
            constexpr size_t VERTEX_COUNT = 8;
            constexpr size_t INDEX_COUNT = 36;

            if (color.m_A == Utils::MAX_CHANNEL_VALUE)
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

        constexpr size_t VERTEX_COUNT = 24;
        constexpr size_t INDEX_COUNT = 36;
        //NOTE: since opaque objects can get depth tested, we can cram as many of them as we want into
        //a batch as long as they have the same state, but for transparent objects
        //they need to have their own batch to ensure correct draw order
        if (color.m_A == Utils::MAX_CHANNEL_VALUE)
        {
            RenderBatch* sameStatebatch = TryGetBatch(shader, texture, INDEX_COUNT);
            if (sameStatebatch != nullptr)
            {
                AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                return;
            }
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
        const Vec2 textureSize = texture->GetData().m_size.AsFloat();
        //The size in texture pixel coords based on its world size
        const Vec3Int pixelSize = CalculateFaceSizeForTexture(size, texture->GetData().m_size);

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

        CreateBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT, modelMatrix, color, true);
    }
    void Renderer::AddCallSphere3DMulti(Shader* shader, Texture* texture, const float radius,
        const Mat4& modelMatrix, const Utils::Color color)
    {
        //Note: this is the default UV method with longitudinal/"slices" (vertical) and latitudinal/"stacks" (horizontal) lines
        constexpr size_t HORIZONTAL_LINE_COUNT = 8;
        //Best shape is formed with 1.5 factor 
        constexpr size_t VERTICAL_LINE_COUNT = HORIZONTAL_LINE_COUNT * 1.5f;
        constexpr size_t TOTAL_VERTEX_COUNT = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT + 2;
        constexpr size_t TOTAL_INDEX_COUNT = (HORIZONTAL_LINE_COUNT - 1) * VERTICAL_LINE_COUNT * 6 + VERTICAL_LINE_COUNT * 6;

        IndexType indices[TOTAL_INDEX_COUNT] = {};
        size_t currIndex = 0;
        
        if (color.m_A == Utils::MAX_CHANNEL_VALUE)
        {
            //Note: the total number of vertices is horizontal-1 * vertical * 6 since we create square
            //for every 2 pairs going downward, thus needing to exlude the final horizontal row
            // + vertical * 3 (north pole)+ vertical*3 (south pole) since each vertical connects with top
            RenderBatch* sameStatebatch = TryGetBatch(shader, texture, TOTAL_INDEX_COUNT);

            if (sameStatebatch != nullptr)
            {
                AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                return;
            }
        }

        RenderBatch& batch= CreateBatch(shader, texture, nullptr, TOTAL_VERTEX_COUNT, nullptr, TOTAL_INDEX_COUNT, modelMatrix, color, false);
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
                //AddVertexToBatch(Vertex{ pos * radius, UV(u, v), pos.GetNormalized() });
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

                //AddIndicesToBatch({ aIndex, cIndex, bIndex });
                //AddIndicesToBatch({ bIndex, cIndex, dIndex });
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
            //AddIndicesToBatch({ aIndex, poleIndex, bIndex });
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
            //AddIndicesToBatch({ aIndex, poleIndex, bIndex });
            AddIndicesToBatch(batch, { aIndex, poleIndex, bIndex });
        }

        FinishBatch(batch);
    }

    void Renderer::AddCallPolygon2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Utils::Color color)
    {
        //TODO: the polygon and circle calls 2d should instead be textures that are drawon on quads to allow for batching
        const float angleStep = 2 * std::numbers::pi / sides;
        const size_t vertexCount = sides + 1;
        const size_t indexCount = sides * 3;

        //m_renderCalls.emplace_back(CircleCall{ centerPos, radius, color });
        if (color.m_A == Utils::MAX_CHANNEL_VALUE)
        {
            //Note: the total number of vertices is horizontal-1 * vertical * 6 since we create square
            //for every 2 pairs going downward, thus needing to exlude the final horizontal row
            // + vertical * 3 (north pole)+ vertical*3 (south pole) since each vertical connects with top
            RenderBatch* sameStatebatch = TryGetBatch(GetCoreShader(CoreShader::Default), nullptr, indexCount);

            if (sameStatebatch != nullptr)
            {
                AddInstanceDataToBatch(*sameStatebatch, modelMatrix, color);
                return;
            }
        }
        
        Vertex* vertices = (Vertex*)alloca(sizeof(Vertex) * vertexCount);
        vertices[0] = {};
        for (size_t i = 1; i < vertexCount; i++)
        {
            vertices[i] = {};
            vertices[i].m_Pos = WorldPosition3D(std::cosf(i * angleStep) * radius, std::sinf(i * angleStep) * radius, 0);
        }

        IndexType* indices = (IndexType*)alloca(sizeof(IndexType) * indexCount);
        for (size_t i = 0; i < sides; i++)
        {
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            //The last vertex index needs to wrap around to start with index 1
            indices[i * 3 + 2] = i < sides - 1 ? i + 2 : 1;
        }
        CreateBatch(GetCoreShader(CoreShader::Default), nullptr, vertices, vertexCount, indices, indexCount, modelMatrix, color, true);
    }
    void Renderer::AddCallCircle2D(const float radius, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallPolygon2D(radius, CIRCLE_SIDE_COUNT, modelMatrix, color);
    }
    void Renderer::AddCallRectangle2D(const Vec2& worldSize, const Mat4& modelMatrix, const Utils::Color& color)
    {
        AddCallRectangle2DMulti(GetCoreShader(CoreShader::Default), nullptr, worldSize, modelMatrix, color);
    }

    void Renderer::AddCallBox3D(const Vec3& size, const Mat4& modelMatrix, const Utils::Color& color)
    {
        AddCallBox3DMulti(GetBaseShader(), nullptr, size, modelMatrix, color);
    }
    void Renderer::AddCallSphere3D(const float radius, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallSphere3DMulti(GetBaseShader(), nullptr, radius, modelMatrix, color);
    }

    void Renderer::AddCallTexture2D(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Utils::Color color)
    {
        //AddRectangleCall2DMulti(GetTextureShader(), &tex, worldSize, modelMatrix, color);
        AddCallRectangle2DMulti(GetBaseTextureShader(), &tex, worldSize, modelMatrix, color);
        //m_textureData.emplace_back(tex, scale);
        //m_renderCalls.emplace_back(TextureCall{ static_cast<TextureID>(m_textureData.size() - 1), worldPos, color });
    }
    void Renderer::AddCallTextureSphere3D(const float radius, Texture& tex, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallSphere3DMulti(GetBaseTextureShader(), &tex, radius, modelMatrix, color);
    }
    void Renderer::AddCallTextureBox3D(const Vec3& size, Material& material,const Mat4& modelMatrix)
    {
        AddCallBox3DMulti(GetBaseTextureShader(), GetMaterialAlbedo(material), size, modelMatrix, material.m_BaseColor);
    }

    void Renderer::AddCallText(const WorldPosition3D& worldPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
    {
        m_textData.emplace_back(font, text, size, spacing);
        m_renderCalls.emplace_back(TextCall{ static_cast<TextID>(m_textData.size() - 1), worldPos, color });
    }
    void Renderer::AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Utils::Color color)
    {
        m_renderCalls.emplace_back(LineCall{ startPos, thickness, length, color });
    }
    void Renderer::AddRectangleLineCall(const WorldPosition3D& worldPos, const float thickness, const Vec2& size, const Utils::Color color)
    {
        m_renderCalls.emplace_back(RectLineCall{ worldPos, thickness, size, color });
    }

    //void Renderer::AddCallPointLight(const WorldPosition3D& worldPos, const Utils::Color color, const float radius)
    //{
    //    if (m_uniformData.m_LightBlock.m_PointLightsCount >= MAX_POINT_LIGHTS)
    //    {
    //        LogError(std::format("Attempted to add point light call at:{} colored:{} "
    //            "but max points lights have been reached", worldPos.ToString(), color.ToString()));
    //        return;
    //    }
    //    m_uniformData.m_LightBlock.m_PointLights[m_uniformData.m_LightBlock.m_PointLightsCount] = PointLightData(worldPos, color.GetNormalized(), radius);
    //    m_uniformData.m_LightBlock.m_PointLightsCount++;

    //    if (DRAW_LIGHT_AREAS)
    //    {
    //        AddCallTextureSphere3D(std::min(0.1f*radius, 1.0f), *GetBaseAlbedo(), CalculateModelMatrix(nullptr, worldPos,
    //            Vec3::One(), Quat::Identity()), LIGHT_AREA_COLOR== LIGHT_AREA_COLOR_FROM_LIGHT? color : LIGHT_AREA_COLOR);
    //    }
    //}

    void Renderer::AddCallPointLight(const WorldPosition3D& worldPos, const Quat& worldRot, const float radius, const Utils::Color color)
    {
        if (m_uniformData.m_LightBlock.m_PointLightsCount >= MAX_POINT_LIGHTS)
        {
            LogError(std::format("Attempted to add point light call at:{} colored:{} "
                "but max points lights have been reached", worldPos.ToString(), color.ToString()));
            return;
        }

        auto& pointlightData = m_uniformData.m_LightBlock.m_PointLights[m_uniformData.m_LightBlock.m_PointLightsCount];
        pointlightData = PointLightData(worldPos, color.GetNormalized(), radius);
        pointlightData.m_ShadowMapIndex = m_uniformData.m_LightBlock.m_PointLightsCount;
        m_uniformData.m_ExtraPointLightData[m_uniformData.m_LightBlock.m_PointLightsCount] = ExtraPointLightData{worldRot};
        m_uniformData.m_LightBlock.m_PointLightsCount++;

        if (DRAW_LIGHT_AREAS)
        {
            AddCallSphere3DMulti(GetCoreShader(CoreShader::Texture), GetBaseAlbedo(), std::min(0.1f * radius, 1.0f), CalculateModelMatrix(nullptr, worldPos,
                Vec3::One(), Quat::Identity()), LIGHT_AREA_COLOR == LIGHT_AREA_COLOR_FROM_LIGHT ? color : LIGHT_AREA_COLOR);
        }
    }
    void Renderer::AddCallDirectionalLight(const Vec3& dir, const Utils::Color color)
    {
        m_uniformData.m_LightBlock.m_DirLight = DirectionalLightData(dir, color.GetNormalized());
    }

    void Renderer::PushCallsToBuffer(const std::vector<RenderCall>& calls)
    {
        for (const auto& call : calls)
            m_renderCalls.push_back(call);
    }
    void Renderer::MoveCallsToBuffer(std::vector<RenderCall>& calls)
    {
        for (auto& call : calls)
            m_renderCalls.emplace_back(std::move(call));
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
            LogError(std::format("Attempted to write view matrix to uniform buffer but failed"));
            return;
        }
        if (!m_viewerUniformBuffer.TryWriteData("projectionMatrix", sizeof(Mat4),
            projMatrix.GetMemPointer()))
        {
            LogError(std::format("Attempted to write projection matrix to uniform buffer but failed"));
            return;
        }
    }
    void Renderer::DrawBatch(RenderBatch& batch)
    {
        FencedBufferSegment* vertexSegment = nullptr;
        const size_t drawVertexCount = batch.m_Vertices.size();
        m_isRenderStalled = !m_vertexBuffer.TryWriteDataFenced(&batch.m_Vertices[0], drawVertexCount, &vertexSegment);
        if (m_isRenderStalled)
        {
            const std::string message = std::format("Stalling render due to vertex buffer. Allocate more space");
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }

        FencedBufferSegment* indexSegment = nullptr;
        const size_t drawIndexCount = batch.m_VertexIndices.size();
        m_isRenderStalled = !m_indexBuffer.TryWriteDataFenced(&batch.m_VertexIndices[0], drawIndexCount, &indexSegment);
        if (m_isRenderStalled)
        {
            const std::string message = std::format("Stalling render due to index buffer. Allocate more space");
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }

        FencedBufferSegment* instanceSegment = nullptr;
        const size_t drawInstanceCount = batch.m_InstanceData.size();
        m_isRenderStalled = !m_instancedBuffer.TryWriteDataFenced(&batch.m_InstanceData[0], drawInstanceCount, &instanceSegment);
        if (m_isRenderStalled)
        {
            const std::string message = std::format("Stalling render due to instance buffer. Allocate more space");
            if (STALL_LOG_TYPE == LogType::Warning) LogWarning(message);
            else if (STALL_LOG_TYPE == LogType::Error) LogError(message);
            return;
        }

        //When we upload to gpu, we can get rid of cpu side buffer data
        //batch.m_VertexIndices.clear();
        //batch.m_Vertices.clear();
        //batch.m_InstanceData.clear();

       /* LogWarning(std::format("Drawing vertices:{}(b:{}) indices:{}(b:{}) isntances:{}(b:{})", drawVertexCount,
            m_vertexBuffer.GetVertexCapacity(), drawIndexCount, m_indexBuffer.GetIndexCapacity(), drawInstanceCount, m_instancedBuffer.GetVertexCapacity()));*/
        Backend::DrawUploadedIndexBufferInstanced(vertexSegment->m_ByteOffset / m_vertexBuffer.GetElementSize(),
            indexSegment->m_ByteOffset, drawIndexCount, instanceSegment->m_ByteOffset / m_instancedBuffer.GetElementSize(), drawInstanceCount);

        vertexSegment->m_Fence.Insert();
        indexSegment->m_Fence.Insert();
        instanceSegment->m_Fence.Insert();
        /*LogWarning(std::format("After fence insert vertexSeg:{} indexSeg:{} instanceSeg:{}",
            vertexSegment->ToString(), indexSegment->ToString(), instanceSegment->ToString()));*/

        LogWarning(std::format("\nDRAWING CALL \nVertexByteOffset:{} basevertexIndex:{} indexByteOffset:{} drawIndices:{} "
            "instanceByteOffset:{} baseinstanceIndex:{} drawInstances:{}",
            vertexSegment->m_ByteOffset, vertexSegment->m_ByteOffset / m_vertexBuffer.GetElementSize(),
            indexSegment->m_ByteOffset, drawIndexCount, instanceSegment->m_ByteOffset,
            instanceSegment->m_ByteOffset / m_instancedBuffer.GetElementSize(), drawInstanceCount));

        m_frameDrawCalls++;
    }

    void Renderer::ExecuteShadowPass()
    {
        Shader* shadowShader = GetCoreShader(CoreShader::Shadow);
        shadowShader->BindUniformBlockIfNeeded(m_viewerUniformBuffer.GetName(), m_viewerUniformBuffer.GetBindIndex());
        shadowShader->BindActive();
        m_frameBuffer.BindActive();

        std::array<Mat4, 6> lightViewMatrices = {};
        Mat4 lightProjMatrix = {};

        for (size_t i = 0; i < m_uniformData.m_LightBlock.m_PointLightsCount; i++)
        {
            auto& light = m_uniformData.m_LightBlock.m_PointLights[i];
            auto& otherLightData = m_uniformData.m_ExtraPointLightData[i];

            lightViewMatrices =
            {
                CalculateViewMatrix(light.m_Pos, ENGINE_RIGHT_DIR,      -ENGINE_UP_DIR),
                CalculateViewMatrix(light.m_Pos, -ENGINE_RIGHT_DIR,     -ENGINE_UP_DIR),
                CalculateViewMatrix(light.m_Pos, ENGINE_UP_DIR,         ENGINE_FORWARD_DIR),
                CalculateViewMatrix(light.m_Pos, -ENGINE_UP_DIR,        -ENGINE_FORWARD_DIR),
                CalculateViewMatrix(light.m_Pos, ENGINE_FORWARD_DIR,    -ENGINE_UP_DIR),
                CalculateViewMatrix(light.m_Pos, -ENGINE_FORWARD_DIR,   -ENGINE_UP_DIR)
            };
            lightProjMatrix= PlatformMath::CalculatePlatformPerspectiveProjMatrix(Utils::ToRadians(90), 1, SHADOW_NEAR_DISTANCE, light.m_Radius);
            Backend::SetViewport(m_shadowMaps[i].GetData().m_size.m_X, m_shadowMaps[i].GetData().m_size.m_Y);

            //For every single face on cube, we redraw scene from light perspective
            for (size_t j = 0; j < 6; j++)
            {
                m_frameBuffer.SetOutputTextureCube(FrameBufferAttachmentType::Depth, &m_shadowMaps[i], static_cast<TextureCubeFace>(j));
                Backend::ClearDepth();

                SetViewerData(light.m_Pos, lightViewMatrices[j], lightProjMatrix);

                for (size_t k = 0; k < m_batches.size(); k++)
                {
                    auto& batch = m_batches[k]; 
                    if (batch.m_InstanceData.empty())
                        continue;

                    //const auto result = lightProjMatrix * lightViewMatrix * m_batches[i].m_InstanceData[0].m_ModelMatrix * Vec4(light.m_Pos, 0);
                    /*LogWarning(std::format("final ndc pos: {} light proj:{} light view:{}", 
                        result.ToString(), lightProjMatrix.ToString(), lightViewMatrix.ToString()));*/
                    DrawBatch(batch);
                }
            }
        }
        //LogError("FIN");

        shadowShader->UnbindActive();
        m_frameBuffer.UnbindActive();
        //This forces the viewport to be set back to rendering for the window
        m_engineState->m_GraphicsContext.m_Window->ForceSizeUpdate();
    }
    void Renderer::ExecuteLightingAndGeometryPass(const TextureSlotIndex* shadowCubeMapSlots)
    {
        //LogError(std::format("Viewport size:{}", Backend::GetViewportSize().ToString()));
        Backend::ClearBackground();

        const CameraComponent& camera = m_engineState->m_CameraController->GetActiveCamera();
        const CameraPrecalculatedData& cameraData = camera.GetLastUpdateData();
        //TODO: this is still a problem since multiple flushes per frame means multiple updates
        const bool needsViewMatrixUpdate = Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::ViewMatrix);
        const bool needsProjMatrixUpdate = Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::PlatformProjMatrix);

        SetViewerData(camera.GetTransform().GetGlobalPos(), cameraData.m_ViewMatrix, cameraData.m_PlatformProjectionMatrix);
        //m_uniformData.m_CameraUpdatedThisFrame = true;

        //TODO: optimize so we check if light block data changed from last render batch and only then write buffer
        m_lightUniformBuffer.WriteData(0, sizeof(LightBlockData), &m_uniformData.m_LightBlock);

        const Texture* lastBatchTexture = nullptr;
        Shader* lastBatchShader = nullptr;

        for (int i = 0; i < m_batches.size(); i++)
        {
            LogWarning(std::format("flushing batch:{}/{}", i + 1, m_batches.size()));
            auto& batch = m_batches[i];

            //If we have no instance data it means it might be a leftover batch from previous frame
            //that was not cleared
            if (batch.m_InstanceData.empty())
                continue;

            if (batch.m_Shader == nullptr)
            {
                LogError(std::format("Tried to flush current batch in renderer, but batch shader was null"));
                return;
            }

            if (lastBatchShader != nullptr && batch.m_Shader == nullptr)
            {
                lastBatchShader->UnbindActive();
            }
            else if (lastBatchShader == nullptr || lastBatchShader != batch.m_Shader)
            {
                batch.m_Shader->BindUniformBlockIfNeeded(m_viewerUniformBuffer.GetName(), m_viewerUniformBuffer.GetBindIndex());
                batch.m_Shader->BindUniformBlockIfNeeded(m_lightUniformBuffer.GetName(), m_lightUniformBuffer.GetBindIndex());
                batch.m_Shader->BindActive();
            }
            lastBatchShader = batch.m_Shader;

            //TODO: right now the current batch gets it texture slot by removing previous slot
            //but what if we use that texture in a future batch it would be wasteful -> so
            //we have to make sure that no future queued batches have last batch texture before removing
            if (lastBatchTexture != nullptr && lastBatchTexture != batch.m_Texture)
            {
                m_textureController.TryRemoveFromSlot(lastBatchTexture->GetData().m_slotIndex);
            }
            if (batch.m_Texture != nullptr && lastBatchTexture != batch.m_Texture)
            {
                TextureSlotIndex slot = m_textureController.TryAddTextureToAvailableSlot(batch.m_Texture);
                if (!batch.m_Shader->TrySetUniform(UniformType::Sampler2D, TEXTURE_UNIFORM_NAME, &slot))
                    return;
            }
            if (DO_SHADOWS && batch.m_Shader== GetCoreShader(CoreShader::ForwardRender))
            {
                batch.m_Shader->TrySetUniform(UniformType::Bool, SHADOW_TOGGLE_UNIFORM_NAME, &DO_SHADOWS);
                batch.m_Shader->TrySetUniformArray(UniformType::CubeSampler, SHADOW_MAP_UNIFORM_NAME,
                    shadowCubeMapSlots, m_uniformData.m_LightBlock.m_PointLightsCount);
            }
            
            lastBatchTexture = batch.m_Texture;
            //LogWarning(std::format("Flushing batch:{} vertex count:{} index:{}", batch.ToString(), batch.m_Vertices.size(), batch.m_VertexIndices.size()));
            //LogWarning(std::format("Drawing normal matrix:{}", batch.m_InstanceData[0].m_NormalModelMatrix.ToString()));

            DrawBatch(batch);
        }

        if (lastBatchShader != nullptr) lastBatchShader->UnbindActive();
        if (lastBatchTexture != nullptr) m_textureController.TryRemoveFromSlot(lastBatchTexture->GetData().m_slotIndex);
    } 

    void Renderer::FlushBatches()
    {
        if (RENDER_FRAMES_COUNT != NO_RENDER_FRAME_COUNT_LIMIT &&
            m_framesSinceStart >= RENDER_FRAMES_COUNT)
            return;

        //TODO: this should ideally be removed and part of the uniform initializer
        if (!m_viewerUniformBuffer.IsAllocated())
        {
            m_viewerUniformBuffer.AllocateFromShaderUniformBlock(*GetCoreShader(CoreShader::ForwardRender), "ViewerBlock");
            m_bufferController.AddUniformBuffer(&m_viewerUniformBuffer);
        }
        if (!m_lightUniformBuffer.IsAllocated())
        {
            m_lightUniformBuffer.AllocateFromShaderUniformBlock(*GetCoreShader(CoreShader::ForwardRender), "LightsBlock");
            m_bufferController.AddUniformBuffer(&m_lightUniformBuffer);
        }

        if (DO_SHADOWS) ExecuteShadowPass();

        //LogError(std::format("Draw calls shadow pass:{} lights:{} batches:{}", m_frameDrawCalls, m_uniformData.m_LightBlock.m_PointLightsCount, m_batches.size()));
        //LogError(std::format("After shadow pass tu:{}", m_textureController.ToString()));

        const size_t totalPointLights = m_uniformData.m_LightBlock.m_PointLightsCount;
        const std::vector<TextureSlotIndex> shadowCubeMapSlots= 
            m_textureController.TryAddTextureCubesToAvailableSlots(m_shadowMaps, totalPointLights);
        //LogError(std::format("shadow slots: {} lihghts:{}", Utils::ToStringIterable<std::vector<//>, TextureSlotIndex>(shadowCubeMapSlots), totalPointLights));
        if (shadowCubeMapSlots.empty() || shadowCubeMapSlots.size()!= totalPointLights)
        {
            LogError(std::format("Attempted to add shadow map textures to available slots but failed." 
                "Reserved slots:{} expected size:{}", shadowCubeMapSlots.size(), totalPointLights));
            return;
        }

        ExecuteLightingAndGeometryPass(&shadowCubeMapSlots[0]);

        m_textureController.RemoveFromSlots(shadowCubeMapSlots[0], shadowCubeMapSlots.size());
    }

    void Renderer::RenderBuffer()
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        if (!m_batches.empty())
        {
            RenderStartActions();
            FlushBatches();
            //LogError(std::format("all batches:{}", ToStringBatches()));

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

        ClearCommandBuffers();
        m_uniformData.m_CameraUpdatedThisFrame = false;
        m_uniformData.m_LightBlock.m_PointLightsCount = 0;
        m_frameDrawCalls = 0;

        m_framesSinceStart++;
    }

    void Renderer::ClearCommandBuffers()
    {
        m_textData.clear();
        m_textureData.clear();
        m_renderCalls.clear();
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
        return std::format("DUMPING RENDERER DATA:\nCameraState:{}\nBatches:{}", 
            m_engineState->m_CameraController->GetActiveCamera().ToString(), ToStringBatches());
    }
}