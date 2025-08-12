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
#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Rendering/RenderingBackend.hpp"

#include "Utils/Data/ColorConstants.hpp"

namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;
    constexpr size_t PRE_ALLOCATED_SHAPES = 30;
    constexpr size_t PRE_ALLOCATED_INDICES_COUNT = 600;
    constexpr size_t PRE_ALLOCATED_VERTICES_COUNT = 200;
    constexpr size_t CIRCLE_SIDE_COUNT = 12;

    constexpr const char* VIEW_MATRIX_UNIFORM_NAME = "uViewMatrix";
    constexpr const char* PROJ_MATRIX_UNIFORM_NAME = "uProjectionMatrix";
    constexpr const char* TEXTURE_UNIFORM_NAME = "uTexture";

    std::string Vertex::ToString() const
    {
        return std::format("[{}]", m_Pos.ToString());
    }
    std::string InstanceData::ToString() const
    {
        return std::format("[Color:{} ModelMatrix:{}]", m_Color.ToString(), m_ModelMatrix.ToString());
    }

    std::string RenderBatch::ToString() const
    {
        return std::format("[Batch Shader:{} Texture:{}Vertices:{} Indices:{} Instances:{}]", 
            m_Shader!=nullptr, m_Texture!=nullptr,
            Utils::ToStringIterable<std::vector<VertexType>, VertexType>(m_Vertices),
            Utils::ToStringIterable<std::vector<IndexType>, IndexType>(m_VertexIndices),
            Utils::ToStringIterable<std::vector<InstanceType>, InstanceType>(m_InstanceData));
    }

    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants

    Renderer::Renderer(const EngineState& engineState)
        : m_isInit(false), m_engineState(&engineState), m_uniformData(), //m_staticRenderData(),
        m_renderCalls(), m_textData(), m_textureData(), m_batches(), m_flushType(BatchFlushType::StateChange), 
        m_layout(), m_bufferController(&m_layout), m_textureController(),
        m_vertexBuffer(), m_indexBuffer(), m_instancedBuffer(), m_uniformBuffer()
    {
        
    }

    void Renderer::Init()
    {
        //We reserve one for current batch, but also keep it as vector for future in case we do rendering in one go
        m_batches.reserve(1);

        m_layout = Backend::CreateVertexLayout();
        m_vertexBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(VertexType), PRE_ALLOCATED_VERTICES_COUNT, VertexAttributeAdvance::Vertex);
        m_indexBuffer = Backend::CreateIndexBuffer(nullptr, PRE_ALLOCATED_INDICES_COUNT);
        m_instancedBuffer = Backend::CreateVertexBuffer(nullptr, sizeof(InstanceData), PRE_ALLOCATED_SHAPES, VertexAttributeAdvance::Instance);
        m_uniformBuffer = Backend::CreateUniformBuffer();

        const VertexLayoutBindIndex vertexBindIndex = m_bufferController.AddVertexBuffer(&m_vertexBuffer, &m_indexBuffer);
        std::vector<VertexAttribute> vertexAttributes = { VertexAttribute(0, 3, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_Pos)), 
                                                          VertexAttribute(1, 2, VertexAttributeBaseType::Float, false, offsetof(VertexType, m_UVPos)) };
        m_bufferController.AddVertexBufferAttributes(vertexBindIndex, vertexAttributes);

        const VertexLayoutBindIndex instancedBindIndex = m_bufferController.AddVertexBuffer(&m_instancedBuffer, nullptr);
        std::vector<VertexAttribute> instancedAttributes = {
            VertexAttribute(2, 4, VertexAttributeBaseType::Float, false, offsetof(InstanceData, m_Color)) };
        m_bufferController.AddVertexBufferAttributes(instancedBindIndex, instancedAttributes);
        m_bufferController.AddVertexBufferMatrix4Attribute(instancedBindIndex, 3, false, sizeof(Vec4), offsetof(InstanceData, m_ModelMatrix));

        m_isInit = true;
    }
    bool Renderer::WasInit() const
    {
        return m_isInit;
    }

    /*
    void RenderBuffer(const FragmentedTextBuffer* buffer, const ColliderOutlineBuffer* outlineBuffer,
        const LineBuffer* lineBuffer, GUIHierarchy* hierarchy)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        BeginDrawing();

        ClearBackground(BLACK);
        //DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);
        if (DONT_RENDER_NON_UTILS)
        {
            EndDrawing();
            return;
        }

        //LogWarning(std::format("DRAWING MIXED BUFFER: {}", ToString(buffer)));

        if (buffer != nullptr && !buffer->empty())
        {
            char drawStr[2] = { '1', '\0' };
            for (const auto& pos : *buffer)
            {
                drawStr[0] = pos.m_Text.m_Char;
                if (!Assert(RaylibUtils::FontSupportsChar(pos.m_FontData.m_FontType, pos.m_Text.m_Char),
                    std::format("GameRenderer tried to render character: {} but font does not support this character!", Utils::ToString(pos.m_Text.m_Char))))
                    continue;

                //LogWarning(std::format("Drawing text at pos: {}", pos.m_Pos.ToString()));
                //This should be optimized to use rows of text rather going through each text char individiaully
                DrawTextEx(pos.m_FontData.m_FontType, drawStr, RaylibUtils::ToRaylibVector(pos.m_Pos), pos.m_FontData.m_Size, 0, pos.m_Text.m_Color);
            }
        }
        
        if (outlineBuffer != nullptr)
        {
            for (const auto& rectangle : outlineBuffer->m_RectangleBuffer)
            {
                DrawRectangleLines(rectangle.m_Position.m_X, rectangle.m_Position.m_Y, 
                    rectangle.m_Size.XAsInt(), rectangle.m_Size.YAsInt(), EditorStyles::COLLIDER_OUTLINE_COLOR);
              //LogWarning(std::format("Rectangle of sixe: {} is being drawn at; {}", 
              //rectangle.m_Size.ToString(), rectangle.m_Position.ToString()));
            }
        }

        if (lineBuffer != nullptr)
        {
            for (const auto& line : *lineBuffer)
            {
                DrawLine(line.m_StartPos.m_X, line.m_StartPos.m_Y, line.m_EndPos.m_X, line.m_EndPos.m_Y, EditorStyles::LINE_COLOR);
                //LogWarning(std::format("Rectangle of sixe: {} is being drawn at; {}",
                //rectangle.m_Size.ToString(), rectangle.m_Position.ToString()));
            }
        }

        //Draws the center screen indicator
        const float CENTER_CIRCLE_RADIUS = 5;
        Color circleColor = WHITE;
        circleColor.a = 50;
        const WorldPosition centerPos = {(float)SCREEN_WIDTH/2, (float)SCREEN_HEIGHT/2};
        DrawCircle(centerPos.m_X, centerPos.m_Y, CENTER_CIRCLE_RADIUS, circleColor);

        //if (console != nullptr) console->TryRender();
       // if (editor != nullptr) editor->TryRender();
        if (hierarchy!=nullptr) hierarchy->RenderAll();

        EndDrawing();
    }
    */
    Shader* Renderer::GetDefaultShader() const
    {
        return m_engineState->m_GraphicsContext.m_GraphicsManager->GetDefaultShaderMutable();
    }
    Shader* Renderer::GetTextureShader() const
    {
        return m_engineState->m_GraphicsContext.m_GraphicsManager->GetTextureShaderMutable();
    }
    /*
    void Renderer::FrameRenderDataUpdateCheck()
    {
        if (!m_staticRenderData.m_UpdatedDataThisFrame)
        {
            m_staticRenderData.m_CameraData = &m_engineState->m_CameraController->GetActiveCamera().GetLastUpdateData();
            m_staticRenderData.m_UpdatedDataThisFrame = true;
        }
    }
    StaticFrameRenderData& Renderer::GetThisFrameRenderData()
    {
        FrameRenderDataUpdateCheck();
        return m_staticRenderData;
    }
    */

    void Renderer::BatchStateChangeCheck(Shader* shader, Texture* texture)
    {
        bool hasStateChange = !m_batches.empty() && (m_batches.back().m_Shader != shader || m_batches.back().m_Texture != texture);
        if (m_flushType == BatchFlushType::StateChange && hasStateChange)
        {
            FlushBatches();
        }

        if (m_batches.empty() || hasStateChange)
        {
            m_batches.emplace_back(shader, texture);
        }
    }

    void Renderer::AddVerticesToBatch(Shader* shader, Texture* texture,
        const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize)
    {
        BatchStateChangeCheck(shader, texture);
       
        const size_t& firstVertexIndex = m_batches.back().m_IndexOffset;
        //Update the indices to match the start of new index
        for (size_t i = 0; i < indicesSize; i++)
            *(indexArray + i) += firstVertexIndex;

        m_batches.back().m_Vertices.insert(m_batches.back().m_Vertices.begin(),
            vertexArray, vertexArray + vertexSize);
        m_batches.back().m_VertexIndices.insert(m_batches.back().m_VertexIndices.end(),
            indexArray, indexArray + indicesSize);

        LogWarning(std::format("Adding new batch vertices:{}", m_batches.back().ToString()));
    }
    void Renderer::AddVertexToBatch(const Vertex& vertex)
    {
        m_batches.back().m_Vertices.emplace_back(vertex);
    }
    void Renderer::AddIndexToBatch(const IndexType& index)
    {
        m_batches.back().m_VertexIndices.push_back(index + m_batches.back().m_IndexOffset);
    }
    void Renderer::AddIndicesToBatch(const std::array<IndexType, 3>& arr)
    {
        m_batches.back().m_VertexIndices.insert(m_batches.back().m_VertexIndices.end(),
            arr.begin(), arr.end());
    }

    void Renderer::AddInstanceDataToBatch(const Mat4& modelMatrix, const Utils::Color& color)
    {
        m_batches.back().m_InstanceData.emplace_back(color.GetNormalized(), modelMatrix);
        //Note: every time we add new instance data to the batch, we increase the index offset since we know
        //the current model has finished
        m_batches.back().m_IndexOffset = m_batches.back().m_Vertices.size();
        //m_batches.back().m_InstanceData.emplace_back(color.GetNormalized(), Mat4::GetIdentity());
    }

    void Renderer::FlushBatches()
    {
        //We only do this the first time we flush a batch during this frame
        //FrameRenderDataUpdateCheck();

        if (!m_uniformBuffer.IsAllocated())
        {
            m_uniformBuffer.AllocateFromShaderUniformBlock(*GetDefaultShader(), "CameraBlock");
            m_bufferController.AddUniformBuffer(&m_uniformBuffer);
        }
        const CameraPrecalculatedData& cameraData = m_engineState->m_CameraController->GetActiveCamera().GetLastUpdateData();
        //TODO: this is still a problem since multiple flushes per frame means multiple updates
        if (!m_uniformData.m_UpdatedThisFrame && Utils::HasFlagAny(cameraData.m_UpdatedThisFrame, CameraPrecalculatedDataUpdate::PlatformProjMatrix, 
            CameraPrecalculatedDataUpdate::ViewMatrix))
        {
            if (!m_uniformBuffer.TryWriteData("viewMatrix", sizeof(Mat4), cameraData.m_ViewMatrix.GetMemPointer()))
            {
                LogError(std::format("Attempted to write view matrix to uniform buffer but failed"));
                return;
            }
            if (!m_uniformBuffer.TryWriteData("projectionMatrix", sizeof(Mat4), cameraData.m_PlatformProjectionMatrix.GetMemPointer()))
            {
                LogError(std::format("Attempted to write projection matrix to uniform buffer but failed"));
                return;
            }

            m_uniformData.m_UpdatedThisFrame = true;
            //LogError(std::format("Updated camera matrices v:{} p:{}", cameraData.m_ViewMatrix.ToString(), cameraData.m_PlatformProjectionMatrix.ToString()));
        }

        for (auto& batch : m_batches)
        {
            if (batch.m_Shader == nullptr)
            {
                LogError(std::format("Tried to flush current batch in renderer, but batch shader was null"));
                return;
            }
            batch.m_Shader->BindUniformBlockIfNeeded(m_uniformBuffer.GetName(), m_uniformBuffer.GetBindIndex());
            batch.m_Shader->BindActive();
            //LogWarning(std::format("Active program binded:{}", batch.m_Shader->GetId()));
           
            /*
            if (!batch.m_Shader->TrySetUniform(UniformType::Matrix4x4, VIEW_MATRIX_UNIFORM_NAME, 
                m_staticRenderData.m_CameraData->m_ViewMatrix.GetMemPointer()))
                return;
            if (!batch.m_Shader->TrySetUniform(UniformType::Matrix4x4, PROJ_MATRIX_UNIFORM_NAME, 
                m_staticRenderData.m_CameraData->m_PlatformProjectionMatrix.GetMemPointer()))
                return;
                */

            if (batch.m_Texture != nullptr)
            {
                TextureSlotIndex slot= m_textureController.AddTextureToAvailableSlot(batch.m_Texture);
                
                if (!batch.m_Shader->TrySetUniform(UniformType::Sampler2D, TEXTURE_UNIFORM_NAME, &slot))
                    return;

                int textureSlot = 0;
                batch.m_Shader->TryGetUniform(UniformType::Sampler2D, TEXTURE_UNIFORM_NAME, &textureSlot);
                LogWarning(std::format("Getting uniform:{} actual:{}", textureSlot, slot));
            }
            LogWarning(std::format("Flushing batch:{} vertex count:{} index:{}", batch.ToString(), batch.m_Vertices.size(), batch.m_VertexIndices.size()));

            const size_t drawVertexCount = batch.m_Vertices.size();
            const size_t drawIndexCount = batch.m_VertexIndices.size();
            const size_t drawInstanceCount = batch.m_InstanceData.size();
            m_vertexBuffer.WriteData(0, &batch.m_Vertices[0], drawVertexCount);
            m_indexBuffer.WriteData(0, &batch.m_VertexIndices[0], drawIndexCount);
            m_instancedBuffer.WriteData(0, &batch.m_InstanceData[0], drawInstanceCount);

            //When we upload to gpu, we can get rid of cpu side buffer data
            batch.m_VertexIndices.clear();
            batch.m_Vertices.clear();
            batch.m_InstanceData.clear();

            LogWarning(std::format("Drawing vertices:{} indices:{} isntances:{}", drawVertexCount, drawIndexCount, drawInstanceCount));
            Backend::DrawUploadedIndexBufferInstanced(0, drawIndexCount, drawInstanceCount);
            //Backend::DrawUploadedIndexBuffer(0, drawIndexCount);

            m_textureController.ClearAllSlots();
            batch.m_Shader->UnbindActive();
        }
        
        m_batches.clear();
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
        const WorldPosition3D halfSize = Vec3(worldSize / 2, 0);
        //Start with top right vertex, then bottom right, then bottom left, top left
        const Vertex vertices[VERTEX_COUNT] = { Vertex(halfSize, UV(1, 1)), Vertex(halfSize * Vec3(1, -1, 0), UV(1, 0)),
                                     Vertex(halfSize * Vec3(-1, -1, 0), UV(0 ,0)), Vertex(halfSize * Vec3(-1, 1, 0), UV(0, 1)) };

        constexpr size_t INDEX_COUNT = 6;
        IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2 };
        AddVerticesToBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT);
        AddInstanceDataToBatch(modelMatrix, color);
    }
    void Renderer::AddCallBox3DMulti(Shader* shader, Texture* texture, const Vec3& size,
        const Mat4& modelMatrix, const Utils::Color& color)
    {
        if (texture == nullptr)
        {
            constexpr size_t VERTEX_COUNT = 8;
            const WorldPosition3D halfSize = size / 2;
            Vertex vertices[VERTEX_COUNT] = { Vertex(halfSize, UV()),                       Vertex(halfSize * Vec3(1, -1, 1), UV()),
                                              Vertex(halfSize * Vec3(-1, -1, 1), UV()),     Vertex(halfSize * Vec3(-1, 1, 1), UV()),
                                              Vertex(halfSize * Vec3(1, 1, -1), UV()),      Vertex(halfSize * Vec3(1, -1, -1), UV()),
                                              Vertex(halfSize * Vec3(-1, -1, -1), UV()),    Vertex(halfSize * Vec3(-1, 1, -1), UV()) };

            constexpr size_t INDEX_COUNT = 36;
            //Front face, back face, right, left, top, bottom
            IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2,
                                               4, 5, 6, 4, 7, 6,
                                               4, 5, 1, 4, 0, 1,
                                               7, 6, 2, 7, 3, 2,
                                               4, 0, 3, 4, 7, 3,
                                               5, 1, 2, 5, 6, 2 };
            AddVerticesToBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT);
            AddInstanceDataToBatch(modelMatrix, color);
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
        const Vec2 textureSize = Vec2(texture->GetWidth(), texture->GetHeight());
        //The size in texture pixel coords based on its world size
        const Vec3Int pixelSize = CalculateFaceSizeForTexture(size, texture->GetSize());

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
        //FACE EDGE ORDER: top right, bottom right, bottom left, top left
        //NOTE: all vertices are as if you are looking north with forward face in front of you
        constexpr size_t VERTEX_COUNT = 24;
        Vertex vertices[VERTEX_COUNT] = {};
        //FRONT FACE (0, 1, 2, 3)
        vertices[0] = Vertex(edges[0], uvs[5]);
        vertices[1] = Vertex(edges[1], uvs[4]);
        vertices[2] = Vertex(edges[2], uvs[2]);
        vertices[3] = Vertex(edges[3], uvs[3]);
        //BACK FACE (4, 5, 6, 7)
        vertices[4] = Vertex(edges[4], uvs[11]);
        vertices[5] = Vertex(edges[5], uvs[10]);
        vertices[6] = Vertex(edges[6], uvs[12]);
        vertices[7] = Vertex(edges[7], uvs[13]);
        //RIGHT FACE (4, 5, 1, 0)
        vertices[8] = Vertex(edges[4], uvs[11]);
        vertices[9] = Vertex(edges[5], uvs[10]);
        vertices[10] = Vertex(edges[1], uvs[4]);
        vertices[11] = Vertex(edges[0], uvs[5]);
        //LEFT FACE (7, 6, 2, 3)
        vertices[12] = Vertex(edges[7], uvs[1]);
        vertices[13] = Vertex(edges[6], uvs[0]);
        vertices[14] = Vertex(edges[2], uvs[2]);
        vertices[15] = Vertex(edges[3], uvs[3]);
        //TOP FACE (4, 0, 3, 7)
        vertices[16] = Vertex(edges[4], uvs[7]);
        vertices[17] = Vertex(edges[0], uvs[5]);
        vertices[18] = Vertex(edges[3], uvs[3]);
        vertices[19] = Vertex(edges[7], uvs[6]);
        //BOTTOM FACE (5, 1, 2, 6)
        vertices[20] = Vertex(edges[5], uvs[9]);
        vertices[21] = Vertex(edges[1], uvs[4]);
        vertices[22] = Vertex(edges[2], uvs[8]);
        vertices[23] = Vertex(edges[6], uvs[2]);

        constexpr size_t INDEX_COUNT = 36;
        
        IndexType indices[INDEX_COUNT] = 
        { 
            /*FRONT FACE*/ 0,  1,  2,  0,  3,  2, 
            /*BACK FACE*/  4,  5,  6,  4,  7,  6,
            /*RIGHT FACE*/ 8,  9,  10, 8,  11, 10,
            /*LEFT FACE*/  12, 13, 14, 12, 15, 14,
            /*TOP FACE*/   16, 17, 18, 16, 19, 18,
            /*BOTTOM FACE*/5,  1,  2,  5,  6,  2
        };

        AddVerticesToBatch(shader, texture, vertices, VERTEX_COUNT, indices, INDEX_COUNT);
        AddInstanceDataToBatch(modelMatrix, color);
    }
    void Renderer::AddCallSphere3DMulti(Shader* shader, Texture* texture, const float radius,
        const Mat4& modelMatrix, const Utils::Color color)
    {
        BatchStateChangeCheck(shader, texture);

        //TODO: right now we do not have very good uv mapping for spheres-> need to increase verticies at poles for increased precision

        //Note: this is the default UV method with longitudinal/"slices" (vertical) and latitudinal/"stacks" (horizontal) lines
        constexpr size_t HORIZONTAL_LINE_COUNT = 8;
        //Best shape is formed with 1.5 factor 
        constexpr size_t VERTICAL_LINE_COUNT = HORIZONTAL_LINE_COUNT * 1.5f;

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

                AddVertexToBatch(Vertex{ pos * radius, UV(u, v) });
                Vec3 normal = pos.GetNormalized();
                Vec2 uv = Vec2(u, v);

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

                AddIndicesToBatch({ aIndex, cIndex, bIndex });
                AddIndicesToBatch({ bIndex, cIndex, dIndex });
            }
        }

        //First we build the north pole vertex and create the indices
        AddVertexToBatch(Vertex{ Vec3(0.0f, radius, 0.0f), UV(1.0f, 1.0f) });
        IndexType poleIndex = HORIZONTAL_LINE_COUNT * VERTICAL_LINE_COUNT;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = vi;
            bIndex = (aIndex + 1) % VERTICAL_LINE_COUNT;
            AddIndicesToBatch({ aIndex, poleIndex, bIndex });
        }

        //Finally, we connect all the bottom latitude/row verticies to the south pole vertex
        AddVertexToBatch(Vertex{ Vec3(0.0f, -1.5 * radius, 0.0f), UV(0.0f, 0.0f) });
        poleIndex++;
        const IndexType bottomStartIndex = poleIndex - VERTICAL_LINE_COUNT - 1;
        for (size_t vi = 0; vi < VERTICAL_LINE_COUNT; vi++)
        {
            aIndex = bottomStartIndex + vi;
            if (vi < VERTICAL_LINE_COUNT - 1) bIndex = aIndex + 1;
            else bIndex = bottomStartIndex;
            AddIndicesToBatch({ aIndex, poleIndex, bIndex });
        }

        AddInstanceDataToBatch(modelMatrix, color);
    }


    void Renderer::AddCallPolygon2D(const float radius, const size_t sides, const Mat4& modelMatrix, const Utils::Color color)
    {
        //m_renderCalls.emplace_back(CircleCall{ centerPos, radius, color });

        //TODO: the polygon and circle calls 2d should instead be textures that are drawon on quads to allow for batching
        const float angleStep = 2 * std::numbers::pi / sides;
        const size_t vertexCount = sides + 1;
        Vertex* vertices = (Vertex*)alloca(sizeof(Vertex) * vertexCount);
        vertices[0] = {};
        for (size_t i = 1; i < vertexCount; i++)
        {
            vertices[i] = {};
            vertices[i].m_Pos = WorldPosition3D(std::cosf(i * angleStep) * radius, std::sinf(i * angleStep) * radius, 0);
        }

        const size_t indexCount = sides * 3;
        IndexType* indices = (IndexType*)alloca(sizeof(IndexType) * indexCount);
        for (size_t i = 0; i < sides; i++)
        {
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            //The last vertex index needs to wrap around to start with index 1
            indices[i * 3 + 2] = i < sides - 1 ? i + 2 : 1;
        }
        AddVerticesToBatch(GetDefaultShader(), nullptr, vertices, vertexCount, indices, indexCount);
        AddInstanceDataToBatch(modelMatrix, color);
    }
    void Renderer::AddCallCircle2D(const float radius, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallPolygon2D(radius, CIRCLE_SIDE_COUNT, modelMatrix, color);
    }
    void Renderer::AddCallRectangle2D(const Vec2& worldSize, const Mat4& modelMatrix, const Utils::Color& color)
    {
        AddCallRectangle2DMulti(GetDefaultShader(), nullptr, worldSize, modelMatrix, color);
    }

    void Renderer::AddCallBox3D(const Vec3& size, const Mat4& modelMatrix, const Utils::Color& color)
    {
        AddCallBox3DMulti(GetDefaultShader(), nullptr, size, modelMatrix, color);
    }
    void Renderer::AddCallSphere3D(const float radius, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallSphere3DMulti(GetDefaultShader(), nullptr, radius, modelMatrix, color);
    }

    void Renderer::AddCallTexture2D(const Vec2& worldSize, Texture& tex, const Mat4& modelMatrix, const Utils::Color color)
    {
        //AddRectangleCall2DMulti(GetTextureShader(), &tex, worldSize, modelMatrix, color);
        AddCallRectangle2DMulti(GetTextureShader(), &tex, worldSize, modelMatrix, color);
        //m_textureData.emplace_back(tex, scale);
        //m_renderCalls.emplace_back(TextureCall{ static_cast<TextureID>(m_textureData.size() - 1), worldPos, color });
    }
    void Renderer::AddCallTextureSphere3D(const float radius, Texture& tex, const Mat4& modelMatrix, const Utils::Color color)
    {
        AddCallSphere3DMulti(GetTextureShader(), &tex, radius, modelMatrix, color);
    }
    void Renderer::AddCallTextureBox3D(const Vec3& size, Texture& tex, const Mat4& modelMatrix, const Utils::Color& color)
    {
        AddCallBox3DMulti(GetTextureShader(), &tex, size, modelMatrix, color);
    }


    void Renderer::AddTextCall(const WorldPosition3D& worldPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
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

    void Renderer::RenderBuffer()
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        Backend::BeginRenderingMarker();
        Backend::ClearBackground();
        //ClearBackground(BLACK);
        
        //LogWarning(std::format("batch coumt:{}", m_batches.size()));
        /*
        for (const auto& call : m_renderCalls)
        {
            if (const CircleCall* c= std::get_if<CircleCall>(&call))
            {
                Backend::DrawCircle(c->m_Pos, c->m_Radius, c->m_Color);
            }
            else if (const RectCall* c = std::get_if<RectCall>(&call))
            {
                //if (RaylibUtils::ColorEqual(c->m_Color, RED)) LogError(std::format("Drawing red rectangle at:{}", c->m_Pos.ToString()));
                Backend::DrawRectangle(c->m_Pos, c->m_Size, c->m_Color);
            }
            else if (const TextureCall* c = std::get_if<TextureCall>(&call))
            {
                TextureCallData& texData = m_textureData[c->m_Id];
                //Vector2 scale = RaylibUtils::ToRaylibVector(texData.m_Scale);

                const Vec2 texSize = Vec2(texData.m_Tex.GetWidth(), texData.m_Tex.GetHeight());

                //Rectangle source = { 0.0f, 0.0f, texSize.x * Utils::GetSign(scale.x), texSize.y* Utils::GetSign(scale.y)};
                //Vector2 drawPos = RaylibUtils::ToRaylibVector(c->m_Pos);
                //Rectangle dest = { drawPos.x, drawPos.y, texSize.x*std::abs(scale.x), texSize.y*std::abs(scale.y)};
                //DrawTexturePro(texData.m_Tex, source, dest, {0, 0}, 0, c->m_Color);
                Backend::DrawTexture(c->m_Pos, texSize * Abs(texData.m_Scale), Vec2::Zero(), texSize * GetSign(texData.m_Scale), texData.m_Tex, 0, c->m_Color);
            }
            else if (const TextCall* c = std::get_if<TextCall>(&call))
            {
                TextCallData& textData = m_textData[c->m_Id];
                Backend::DrawText(c->m_Pos, textData.m_Font, textData.m_Text, textData.m_FontSize, textData.m_Spacing, c->m_Color);
            }
            else if (const LineCall* c = std::get_if<LineCall>(&call))
            {
                Backend::DrawLine(c->m_Pos, c->m_Pos + c->m_Length, c->m_Thickness, c->m_Color);
            }
            else if (const RectLineCall* c = std::get_if<RectLineCall>(&call))
            {
                Backend::DrawRectangleLine(c->m_Pos, c->m_Thickness, c->m_Size, c->m_Color);
            }
        }
        */
        if (!m_batches.empty())
        {
            //LogError(std::format("Flushing batches at render end: {}", ToStringAll()));
            FlushBatches();
        }
        Backend::EndRenderingMarker();

        ClearCommandBuffers();
        m_uniformData.m_UpdatedThisFrame = false;
    }

    void Renderer::ClearCommandBuffers()
    {
        m_textData.clear();
        m_textureData.clear();
        m_renderCalls.clear();
    }

    std::string Renderer::ToStringBatches() const
    {
        std::string result = "";
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