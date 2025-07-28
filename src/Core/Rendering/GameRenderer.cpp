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
namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;
    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants

    Renderer::Renderer() 
        : m_renderCalls(), m_textData(), m_textureData(), m_batches(), m_currentBatchIndex(-1), m_flushType(BatchFlushType::StateChange)
    {
        //We reserve one for current batch, but also keep it as vector for future in case we do rendering in one go
        m_batches.reserve(1);
    }

    void Renderer::Init()
    {
        Rendering::Backend::LoadBackend();
    }

    void Renderer::CreateBatches()
    {
        for (const auto& call : m_renderCalls)
        {
            if (const CircleCall* c = std::get_if<CircleCall>(&call))
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

    void Renderer::AddVerticesToBatch(const Shader* shader, const Vertex* vertexArray, const size_t vertexSize, IndexType* indexArray, const size_t indicesSize)
    {
        //TODO: add a check if the currrent call has a shader that does not match the current batch
        if (m_batches.empty())
        {
            m_batches.push_back(RenderBatch{ shader, {}, {} });
            m_currentBatchIndex++;
        }

        const size_t firstVertexIndex = m_batches[m_currentBatchIndex].m_Vertices.size();
        m_batches[m_currentBatchIndex].m_Vertices.insert(m_batches[m_currentBatchIndex].m_Vertices.begin(), 
            vertexArray, vertexArray+ vertexSize);

        //Update the indices to match the start of new index
        for (size_t i = 0; i < indicesSize; i++)
            *(indexArray + i) += firstVertexIndex;

        m_batches[m_currentBatchIndex].m_VertexIndices.insert(m_batches[m_currentBatchIndex].m_VertexIndices.end(), 
            indexArray, indexArray+ indicesSize);
    }

    void Renderer::FlushBatches()
    {
        for (const auto& batch : m_batches)
        {
            batch.m_Shader->BindActive();
            Backend::DrawBatch(batch.m_Shader, &batch.m_Vertices[0], batch.m_Vertices.size(), 
                &batch.m_VertexIndices[0], batch.m_VertexIndices.size());
            batch.m_Shader->UnbindActive();
        }
        
        m_currentBatchIndex = -1;
        m_batches.clear();
    }

    void Renderer::AddCircleCall(const WorldPosition3D& centerPos, const float radius, const Utils::Color color)
    {
        m_renderCalls.emplace_back(CircleCall{ centerPos, radius, color });
    }
    void Renderer::AddRectangleCall(const WorldPosition3D& topLeftPos, const Vec2& size, const Utils::Color color)
    {
        m_renderCalls.emplace_back(RectCall{ topLeftPos, size, color });
       
        constexpr size_t VERTEX_COUNT = 4;
        const Vertex vertices[VERTEX_COUNT] = { {topLeftPos}, {topLeftPos + WorldPosition(0, -size.m_Y)},
                                     {topLeftPos + WorldPosition(size.m_X, -size.m_Y)}, {topLeftPos + WorldPosition(size.m_X, 0)} };

        constexpr size_t INDEX_COUNT = 6;
        IndexType indices[INDEX_COUNT] = { 0, 1, 2, 0, 3, 2 };
        AddVerticesToBatch(m_defaultShader, vertices, VERTEX_COUNT, indices, INDEX_COUNT);
    }
    void Renderer::AddTextureCall(const WorldPosition3D& topLeftPos, const Texture& tex, const float rotation, const Vec2 scale, const Utils::Color color)
    {
        m_textureData.emplace_back(tex, scale);
        m_renderCalls.emplace_back(TextureCall{ static_cast<TextureID>(m_textureData.size() - 1), topLeftPos, color });
    }
    void Renderer::AddTextCall(const WorldPosition3D& topLeftPos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
    {
        m_textData.emplace_back(font, text, size, spacing);
        m_renderCalls.emplace_back(TextCall{ static_cast<TextID>(m_textData.size() - 1), topLeftPos, color });
    }
    void Renderer::AddLineCall(const WorldPosition3D& startPos, const float thickness, const Vec2& length, const Utils::Color color)
    {
        m_renderCalls.emplace_back(LineCall{ startPos, thickness, length, color });
    }
    void Renderer::AddRectangleLineCall(const WorldPosition3D& topLeftPos, const float thickness, const Vec2& size, const Utils::Color color)
    {
        m_renderCalls.emplace_back(RectLineCall{ topLeftPos, thickness, size, color });
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

        //LogError(std::format("Frame has: {} render calls", std::to_string(RenderCalls.size())));
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
        if (!m_batches.empty()) FlushBatches();
        Backend::EndRenderingMarker();

        ClearCommandBuffers();
    }

    void Renderer::ClearCommandBuffers()
    {
        m_textData.clear();
        m_textureData.clear();
        m_renderCalls.clear();
    }
}