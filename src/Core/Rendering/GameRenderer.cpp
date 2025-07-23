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
#include "Core/UI/UIHierarchy.hpp"
#include "Editor/EditorStyles.hpp"
#include "RenderingBackend.hpp"


namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;
    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants
    std::vector<RenderCall> RenderCalls;
    std::vector<TextCallData> TextData;
    std::vector<TextureCallData> TextureData;

    Renderer::Renderer() {}

    void Renderer::InitBackend()
    {
        Rendering::Backend::LoadBackend();
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

    void Renderer::AddCircleCall(const ScreenPosition& pos, const float radius, const Utils::Color color)
    {
        RenderCalls.emplace_back(CircleCall{ pos, radius, color });
    }
    void Renderer::AddRectangleCall(const ScreenPosition& pos, const Vec2& size, const Utils::Color color)
    {
        RenderCalls.emplace_back(RectCall{ pos, size, color });
    }
    void Renderer::AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const Vec2 scale, const Utils::Color color)
    {
        TextureData.emplace_back(tex, scale);
        RenderCalls.emplace_back(TextureCall{ static_cast<TextureID>(TextureData.size() - 1), pos, color });
    }
    void Renderer::AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color)
    {
        TextData.emplace_back(font, text, size, spacing);
        RenderCalls.emplace_back(TextCall{ static_cast<TextID>(TextData.size() - 1), pos, color });
    }
    void Renderer::AddLineCall(const ScreenPosition& pos, const float thickness, const Vec2& length, const Utils::Color color)
    {
        RenderCalls.emplace_back(LineCall{ pos, thickness, length, color });
    }
    void Renderer::AddRectangleLineCall(const ScreenPosition& pos, const float thickness, const Vec2& size, const Utils::Color color)
    {
        RenderCalls.emplace_back(RectLineCall{ pos, thickness, size, color });
    }

    void Renderer::PushCallsToBuffer(const std::vector<RenderCall>& calls)
    {
        for (const auto& call : calls)
            RenderCalls.push_back(call);
    }
    void Renderer::MoveCallsToBuffer(std::vector<RenderCall>& calls)
    {
        for (auto& call : calls)
            RenderCalls.emplace_back(std::move(call));
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
        for (const auto& call : RenderCalls)
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
                TextureCallData& texData = TextureData[c->m_Id];
                //Vector2 scale = RaylibUtils::ToRaylibVector(texData.m_Scale);

                const Vec2 texSize = Vec2(texData.m_Tex.GetWidth(), texData.m_Tex.GetHeight());

                //Rectangle source = { 0.0f, 0.0f, texSize.x * Utils::GetSign(scale.x), texSize.y* Utils::GetSign(scale.y)};
                //Vector2 drawPos = RaylibUtils::ToRaylibVector(c->m_Pos);
                //Rectangle dest = { drawPos.x, drawPos.y, texSize.x*std::abs(scale.x), texSize.y*std::abs(scale.y)};
                //DrawTexturePro(texData.m_Tex, source, dest, {0, 0}, 0, c->m_Color);
                Backend::DrawTexture(c->m_Pos, texSize * Abs(texData.m_Scale), Vec2::ZERO, texSize * GetSign(texData.m_Scale), texData.m_Tex, 0, c->m_Color);
            }
            else if (const TextCall* c = std::get_if<TextCall>(&call))
            {
                TextCallData& textData = TextData[c->m_Id];
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
        Backend::EndRenderingMarker();

        ClearCommandBuffers();
    }

    void Renderer::ClearCommandBuffers()
    {
        TextData.clear();
        TextureData.clear();
        RenderCalls.clear();
    }
}