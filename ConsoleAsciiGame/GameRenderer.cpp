//NOT USED
#include "pch.hpp"

#include "GameRenderer.hpp"
#include "TextBuffer.hpp"
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

namespace Rendering
{
    void RenderBuffer(const TextBuffer& buffer)
    {
        //Utils::Log(std::format("Rendering buffer: {}", buffer.ToString()));
        if (!Utils::Assert(buffer.GetWidth() != 0 && buffer.GetHeight() != 0,
            std::format("Tried to render a buffer with GameRenderer that has width and/or height of 0")))
            return;

        BeginDrawing();

        ClearBackground(BLACK);
        DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);
        EndDrawing(); 
        return;

        //TODO: perhaps we should not calculate the best fit char area, but rather have a consistent size to allow
        //different scene sizes to appear the same with character area
        int totalUsableWidth = SCREEN_WIDTH - (2 * TEXT_BUFFER_PADDING.m_X);
        int totalUsableHeight = SCREEN_HEIGHT - (2 * TEXT_BUFFER_PADDING.m_Y);
        Utils::Point2DInt charArea = { totalUsableWidth / buffer.GetWidth(), totalUsableHeight / buffer.GetHeight()};
        /* std::cout << std::format("Out of total screen: W{}x H{} usable is W{} x H{} with char area: w{} h{}",
             std::to_string(SCREEN_WIDTH), std::to_string(SCREEN_HEIGHT), std::to_string(totalUsableWidth), std::to_string(totalUsableHeight),
             std::to_string(charArea.m_X), std::to_string(charArea.m_Y)) << std::endl;*/

        int x = TEXT_BUFFER_PADDING.m_X;
        int y = TEXT_BUFFER_PADDING.m_Y;
        char drawStr[2] = { '1', '\0' };

        for (int r = 0; r < buffer.GetHeight(); r++)
        {
            for (int c = 0; c < buffer.GetWidth(); c++)
            {
                
                drawStr[0] = buffer.GetAt({ r, c })->m_Char;
                if (drawStr[0] != EMPTY_CHAR_PLACEHOLDER)
                {
                    DrawText(drawStr, x, y, TEXT_BUFFER_FONT, buffer.GetAt({ r, c })->m_Color);
                }
               Utils::Log(std::format("Drawing character: {} at pos: {} with color: {}", 
                    Utils::ToString(drawStr[0]), Utils::Point2DInt(r, c).ToString(), RaylibUtils::ToString(buffer.GetAt({ r, c })->m_Color)));
                x += charArea.m_X;
            }

            y += charArea.m_Y;
            x = TEXT_BUFFER_PADDING.m_X;
        }

        EndDrawing();
    }
}