//NOT USED
#include "pch.hpp"
#include <optional>
#include "Globals.hpp"

#include "GameRenderer.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "ProfilerTimer.hpp"
#include "Debug.hpp"

namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;

    void RenderBuffer(const TextBufferMixed& buffer, const RenderInfo& renderInfo)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 

       // //Log(std::format("Rendering buffer: {}", buffer.ToString()));
       // if (!Assert(buffer.GetWidth() != 0 && buffer.GetHeight() != 0,
       //     std::format("Tried to render a buffer with GameRenderer that has width and/or height of 0")))
       //     return;

       // BeginDrawing();

       // ClearBackground(BLACK);
       // RaylibUtils::DrawFPSCounter();
       ///* DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);*/
       // if (DONT_RENDER_NON_UTILS)
       // {
       //     EndDrawing();
       //     return;
       // }
       //
       // //TODO: perhaps we should not calculate the best fit char area, but rather have a consistent size to allow
       // //different scene sizes to appear the same with character area
       // int totalUsableWidth = renderInfo.m_ScreenSize.m_X - (2 * renderInfo.m_Padding.m_X);
       // int totalUsableHeight = renderInfo.m_ScreenSize.m_Y - (2 * renderInfo.m_Padding.m_Y);

       // Utils::Point2DInt charArea;
       // if (renderInfo.m_CharSpacing.has_value()) charArea = renderInfo.m_CharSpacing.value();
       // else charArea = { totalUsableWidth / buffer.GetWidth(), totalUsableHeight / buffer.GetHeight() };
       // /* std::cout << std::format("Out of total screen: W{}x H{} usable is W{} x H{} with char area: w{} h{}",
       //      std::to_string(SCREEN_WIDTH), std::to_string(SCREEN_HEIGHT), std::to_string(totalUsableWidth), std::to_string(totalUsableHeight),
       //      std::to_string(charArea.m_X), std::to_string(charArea.m_Y)) << std::endl;*/

       // int x = renderInfo.m_Padding.m_X;
       // int y = renderInfo.m_Padding.m_Y;
       // if (renderInfo.m_CenterBuffer)
       // {
       //     int widthLeft = totalUsableWidth - (charArea.m_X* buffer.GetWidth());
       //     int heightLeft = totalUsableHeight - (charArea.m_Y*buffer.GetHeight());

       //     x += (widthLeft/2);
       //     y += (heightLeft /2);
       // }
       // //Log(std::format("CHAR AREA: {}", charArea.ToString()));

       // char drawStr[2] = { '1', '\0' };
       //
       // //TODO: instead of drawing text directly and separeately maybe look into
       // //having the buffer be a texture and drawing the text there rather and then redenderign the texture each frame
       // //since text is not optimized very well in raylib
       // int startX = x;
       // for (int r = 0; r < buffer.GetHeight(); r++)
       // {
       //     for (int c = 0; c < buffer.GetWidth(); c++)
       //     {
       //         const TextChar& currentChar = buffer.GetAtUnsafe({ r, c });
       //         drawStr[0] = currentChar.m_Char;
       //         if (drawStr[0] != EMPTY_CHAR_PLACEHOLDER)
       //         {
       //             DrawText(drawStr, x, y, renderInfo.m_FontSize, currentChar.m_Color);
       //         }
       //        /*Log(std::format("Drawing character: {} at pos: {} with color: {}", 
       //             Utils::ToString(drawStr[0]), Utils::Point2DInt(r, c).ToString(), RaylibUtils::ToString(buffer.GetAt({ r, c })->m_Color)));*/
       //         x += charArea.m_X;
       //     }

       //     y += charArea.m_Y;
       //     x = startX;
       // }

       // EndDrawing();
    }

    void RenderBuffer(const TextBufferMixed& buffer, const ColliderOutlineBuffer* outlineBuffer, 
        const LineBuffer* lineBuffer, const DebugInfo* debugInfo, const CommandConsole* console)
    {
#ifdef ENABLE_PROFILER
        ProfilerTimer timer("GameRenderer::RenderBuffer");
#endif 
        BeginDrawing();

        ClearBackground(BLACK);
        /* DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);*/
        if (DONT_RENDER_NON_UTILS)
        {
            EndDrawing();
            return;
        }

        //LogWarning(std::format("DRAWING MIXED BUFFER: {}", ToString(buffer)));

        char drawStr[2] = { '1', '\0' };
        for (const auto& pos : buffer)
        {
            drawStr[0] = pos.m_Text.m_Char;
            if (!Assert(RaylibUtils::FontSupportsChar(*pos.m_FontData.m_Font, pos.m_Text.m_Char), 
                std::format("GameRenderer tried to render character: {} but font does not support this character!", Utils::ToString(pos.m_Text.m_Char))))
                continue;

            //LogWarning(std::format("Drawing text at pos: {}", pos.m_Pos.ToString()));
            //This should be optimized to use rows of text rather going through each text char individiaully
            DrawTextEx(*(pos.m_FontData.m_Font), drawStr, RaylibUtils::ToRaylibVector(pos.m_Pos), pos.m_FontData.m_FontSize, 0, pos.m_Text.m_Color);
        }

        if (outlineBuffer != nullptr)
        {
            for (const auto& rectangle : outlineBuffer->m_RectangleBuffer)
            {
                DrawRectangleLines(rectangle.m_Position.m_X, rectangle.m_Position.m_Y, 
                    rectangle.m_Size.XAsInt(), rectangle.m_Size.YAsInt(), COLLIDER_OUTLINE_COLOR);
              /*  LogWarning(std::format("Rectangle of sixe: {} is being drawn at; {}", 
                    rectangle.m_Size.ToString(), rectangle.m_Position.ToString()));*/
            }
        }

        if (lineBuffer != nullptr)
        {
            for (const auto& line : *lineBuffer)
            {
                DrawLine(line.m_StartPos.m_X, line.m_StartPos.m_Y, line.m_EndPos.m_X, line.m_EndPos.m_Y, LINE_COLOR);
                /*  LogWarning(std::format("Rectangle of sixe: {} is being drawn at; {}",
                      rectangle.m_Size.ToString(), rectangle.m_Position.ToString()));*/
            }
        }

        if (debugInfo != nullptr)
        {
            Vector2 startPos = {5, 5};
            Vector2 currentPos = startPos;
            Vector2 currentSize = {};

            Color defaultColor = DEBUG_TEXT_COLOR;
            Color currentColor = defaultColor;

            const auto& highlightedIndices = debugInfo->GetHighlightedIndicesSorted();
            if (!highlightedIndices.empty()) defaultColor.a = 100;

            size_t currentIndex = 0;
            size_t currentCollectionIndex = 0;
            size_t nextHighlightedIndex = !highlightedIndices.empty() ? highlightedIndices[currentCollectionIndex] : -1;

            for (const auto& text : debugInfo->GetText())
            {
                if (!highlightedIndices.empty() && currentIndex == nextHighlightedIndex)
                {
                    currentColor = DEBUG_HIGHLIGHTED_TEXT_COLOR;
                    currentCollectionIndex++;
                    if (currentCollectionIndex < highlightedIndices.size())
                    {
                        nextHighlightedIndex = highlightedIndices[currentCollectionIndex];
                    }
                }
                else currentColor = defaultColor;

                currentSize = MeasureTextEx(GetGlobalFont(), text.c_str(), DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X);
                DrawTextEx(GetGlobalFont(), text.c_str(), currentPos, DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, currentColor);

                currentPos.y += currentSize.y + DEBUG_INFO_CHAR_SPACING.m_Y;
                currentIndex++;
            }

           auto maybeMouseData = debugInfo->GetMouseDebugData();
            if (maybeMouseData.has_value())
            {
                std::string textAsStr = std::format("{}", maybeMouseData.value().m_MouseWorldPos.ToString(2));
                const char* text = textAsStr.c_str();
                DrawTextEx(GetGlobalFont(), text, RaylibUtils::ToRaylibVector(maybeMouseData.value().m_MouseTextScreenPos), 
                    DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, WHITE);

            } 
        }

        //Draws the center screen indicator
        const float CENTER_CIRCLE_RADIUS = 5;
        Color circleColor = WHITE;
        circleColor.a = 50;
        const WorldPosition centerPos = {(float)SCREEN_WIDTH/2, (float)SCREEN_HEIGHT/2};
        DrawCircle(centerPos.m_X, centerPos.m_Y, CENTER_CIRCLE_RADIUS, circleColor);

        if (console != nullptr)
        {
            float consoleIndent = 10;
            Vector2 currentPos = {0, SCREEN_HEIGHT - COMMAND_CONSOLE_HEIGHT };
            Color consoleColor = GRAY;
            consoleColor.a = 100;
            DrawRectangle(currentPos.x, currentPos.y, SCREEN_WIDTH, COMMAND_CONSOLE_HEIGHT, consoleColor);

            DrawTextEx(GetGlobalFont(), console->GetInput().c_str(), {currentPos.x+consoleIndent, currentPos.y}, COMMAND_CONSOLE_FONT_SIZE, COMMAND_CONSOLE_SPACING, WHITE);
            currentPos.y -= COMMAND_CONSOLE_HEIGHT;

            const auto& outputMessages = console->GetOutputMessages();
            Vector2 currentMessageSize = {};
            for (int i = 0; i < outputMessages.size(); i++)
            {
                currentMessageSize = MeasureTextEx(GetGlobalFont(), outputMessages[i].m_Message.c_str(), COMMAND_CONSOLE_OUPUT_FONT_SIZE, COMMAND_CONSOLE_SPACING);
                DrawTextEx(GetGlobalFont(), outputMessages[i].m_Message.c_str(), { currentPos.x + consoleIndent, currentPos.y }, 
                    COMMAND_CONSOLE_OUPUT_FONT_SIZE, COMMAND_CONSOLE_SPACING, outputMessages[i].m_Color);
                currentPos.y -= currentMessageSize.y;
            }
        }

        EndDrawing();
    }
}