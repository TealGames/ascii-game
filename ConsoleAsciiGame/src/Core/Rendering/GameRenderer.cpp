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


namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;
    //TODO: since rendering needs to be fast, optmize render calls with void* instead of variants
    std::vector<RenderCall> RenderCalls;
    std::vector<TextCallData> TextData;
    std::vector<TextureCallData> TextureData;

    Renderer::Renderer() {}

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

    void Renderer::AddCircleCall(const ScreenPosition& pos, const float radius, const Color color)
    {
        RenderCalls.emplace_back(CircleCall{ pos, radius, color });
    }
    void Renderer::AddRectangleCall(const ScreenPosition& pos, const Vec2& size, const Color color)
    {
        RenderCalls.emplace_back(RectCall{ pos, size, color });
    }
    void Renderer::AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const Vec2 scale, const Color color)
    {
        TextureData.emplace_back(tex, scale);
        RenderCalls.emplace_back(TextureCall{ static_cast<TextureID>(TextureData.size() - 1), pos, color });
    }
    void Renderer::AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Color color)
    {
        TextData.emplace_back(font, text, size, spacing);
        RenderCalls.emplace_back(TextCall{ static_cast<TextID>(TextData.size() - 1), pos, color });
    }
    void Renderer::AddLineCall(const ScreenPosition& pos, const float thickness, const Vec2& length, const Color color)
    {
        RenderCalls.emplace_back(LineCall{ pos, thickness, length, color });
    }
    void Renderer::AddRectangleLineCall(const ScreenPosition& pos, const float thickness, const Vec2& size, const Color color)
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
        BeginDrawing();
        ClearBackground(BLACK);

        //LogError(std::format("Frame has: {} render calls", std::to_string(RenderCalls.size())));
        for (const auto& call : RenderCalls)
        {
            if (const CircleCall* c= std::get_if<CircleCall>(&call))
            {
                DrawCircle(c->m_Pos.m_X, c->m_Pos.m_Y, c->m_Radius, c->m_Color);
            }
            else if (const RectCall* c = std::get_if<RectCall>(&call))
            {
                //if (RaylibUtils::ColorEqual(c->m_Color, RED)) LogError(std::format("Drawing red rectangle at:{}", c->m_Pos.ToString()));
                DrawRectangle(c->m_Pos.m_X, c->m_Pos.m_Y, c->m_Size.m_X, c->m_Size.m_Y, c->m_Color);
            }
            else if (const TextureCall* c = std::get_if<TextureCall>(&call))
            {
                TextureCallData& texData = TextureData[c->m_Id];
                Vector2 scale = RaylibUtils::ToRaylibVector(texData.m_Scale);
                Vector2 texSize = { texData.m_Tex.width, texData.m_Tex.height };
                //(std::format("Scale is:{}", texData.m_Scale.ToString()));
                /*LogError(std::format("Rendering texre at pos:{} scale:{} tex id: {} rotation:{} color:{}", 
                    c->m_Pos.ToString(), texData.m_Scale, texData.m_Tex.id, texData.m_Rotation, RaylibUtils::ToString(c->m_Color)));*/

                //DrawTextureEx(texData.m_Tex, RaylibUtils::ToRaylibVector(c->m_Pos), texData.m_Rotation, texData.m_Scale, c->m_Color);
                Rectangle source = { 0.0f, 0.0f, (float)texSize.x * Utils::GetSign(scale.x), (float)texSize.y* Utils::GetSign(scale.y)};
                //LogWarning(std::format("source size:{} {}", source.width, source.height));
                Vector2 drawPos = RaylibUtils::ToRaylibVector(c->m_Pos);
                Rectangle dest = { drawPos.x, drawPos.y, texSize.x*std::abs(scale.x), texSize.y*std::abs(scale.y)};
                DrawTexturePro(texData.m_Tex, source, dest, {0, 0}, 0, c->m_Color);
            }
            else if (const TextCall* c = std::get_if<TextCall>(&call))
            {
                TextCallData& textData = TextData[c->m_Id];
                DrawTextEx(textData.m_Font, textData.m_Text, RaylibUtils::ToRaylibVector(Vec2Int(c->m_Pos.m_X, c->m_Pos.m_Y)), textData.m_FontSize, textData.m_Spacing, c->m_Color);
            }
            else if (const LineCall* c = std::get_if<LineCall>(&call))
            {
                Vector2 startPos = RaylibUtils::ToRaylibVector(c->m_Pos);
                DrawLineEx(startPos, Vector2{ startPos.x + c->m_Length.m_X, startPos.y + c->m_Length.m_Y }, c->m_Thickness, c->m_Color);
            }
            else if (const RectLineCall* c = std::get_if<RectLineCall>(&call))
            {
                DrawRectangleLinesEx(Rectangle{ c->m_Pos.m_X, c->m_Pos.m_Y, c->m_Size.m_X, c->m_Size.m_Y }, c->m_Thickness, c->m_Color);
            }
        }
        EndDrawing();

        ClearBuffer();
    }

    void Renderer::ClearBuffer()
    {
        TextData.clear();
        TextureData.clear();
        RenderCalls.clear();
    }
}