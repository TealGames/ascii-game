//NOT USED
#include "pch.hpp"
#include <optional>
#include <queue>
#include "Globals.hpp"

#include "GameRenderer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ProfilerTimer.hpp"
#include "Debug.hpp"
#include "GUIHierarchy.hpp"
#include "EditorStyles.hpp"

namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;
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
        RenderCalls.push_back(CircleCall{ pos, radius, color });
    }
    void Renderer::AddRectangleCall(const ScreenPosition& pos, const Vec2Int& size, const Color color)
    {
        RenderCalls.push_back(RectCall{ pos, size, color });
    }
    void Renderer::AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const float scale, const Color color)
    {
        TextureData.emplace_back(tex, rotation, scale);
        RenderCalls.push_back(TextureCall{ pos, static_cast<TextureID>(TextureData.size()-1), color});
    }
    void Renderer::AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Color color)
    {
        TextData.emplace_back(font, text, size, spacing);
        RenderCalls.push_back(TextCall{ pos, static_cast<TextID>(TextData.size() - 1), color });
    }

    void Renderer::RenderQueue()
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        BeginDrawing();
        for (const auto& call : RenderCalls)
        {
            if (const CircleCall* c= std::get_if<CircleCall>(&call))
            {
                DrawCircle(c->m_Pos.m_X, c->m_Pos.m_Y, c->m_Radius, c->m_Color);
            }
            else if (const RectCall* c = std::get_if<RectCall>(&call))
            {
                DrawRectangle(c->m_Pos.m_X, c->m_Pos.m_Y, c->m_Size.m_X, c->m_Pos.m_Y, c->m_Color);
            }
            else if (const TextureCall* c = std::get_if<TextureCall>(&call))
            {
                TextureCallData& texData = TextureData[c->m_Id];
                DrawTextureEx(texData.m_Tex, RaylibUtils::ToRaylibVector(c->m_Pos), texData.m_Rotation, texData.m_Scale, c->m_Color);
            }
            else if (const TextCall* c = std::get_if<TextCall>(&call))
            {
                TextCallData& textData = TextData[c->m_Id];
                DrawTextEx(textData.m_Font, textData.m_Text, RaylibUtils::ToRaylibVector(c->m_Pos), textData.m_FontSize, textData.m_Spacing, c->m_Color);
            }   
        }
        EndDrawing();
    }
}