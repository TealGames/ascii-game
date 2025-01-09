//NOT USED
#include "pch.hpp"
#include <optional>

#include "GameRenderer.hpp"
#include "TextBuffer.hpp"
//#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"

namespace Rendering
{
    constexpr bool DONT_RENDER_NON_UTILS = false;

    void RenderBuffer(const TextBuffer& buffer, const RenderInfo& renderInfo)
    {
        //Utils::Log(std::format("Rendering buffer: {}", buffer.ToString()));
        if (!Utils::Assert(buffer.GetWidth() != 0 && buffer.GetHeight() != 0,
            std::format("Tried to render a buffer with GameRenderer that has width and/or height of 0")))
            return;

        BeginDrawing();

        ClearBackground(BLACK);
        DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);
        if (DONT_RENDER_NON_UTILS)
        {
            EndDrawing();
            return;
        }
       
        //TODO: perhaps we should not calculate the best fit char area, but rather have a consistent size to allow
        //different scene sizes to appear the same with character area
        int totalUsableWidth = renderInfo.m_ScreenSize.m_X - (2 * renderInfo.m_Padding.m_X);
        int totalUsableHeight = renderInfo.m_ScreenSize.m_Y - (2 * renderInfo.m_Padding.m_Y);

        Utils::Point2DInt charArea;
        if (renderInfo.m_CharSpacing.has_value()) charArea = renderInfo.m_CharSpacing.value();
        else charArea = { totalUsableWidth / buffer.GetWidth(), totalUsableHeight / buffer.GetHeight() };
        /* std::cout << std::format("Out of total screen: W{}x H{} usable is W{} x H{} with char area: w{} h{}",
             std::to_string(SCREEN_WIDTH), std::to_string(SCREEN_HEIGHT), std::to_string(totalUsableWidth), std::to_string(totalUsableHeight),
             std::to_string(charArea.m_X), std::to_string(charArea.m_Y)) << std::endl;*/

        int x = renderInfo.m_Padding.m_X;
        int y = renderInfo.m_Padding.m_Y;
        if (renderInfo.m_CenterBuffer)
        {
            int widthLeft = totalUsableWidth - (charArea.m_X* buffer.GetWidth());
            int heightLeft = totalUsableHeight - (charArea.m_Y*buffer.GetHeight());

            x += (widthLeft/2);
            y += (heightLeft /2);
        }
        //Utils::Log(std::format("CHAR AREA: {}", charArea.ToString()));

        char drawStr[2] = { '1', '\0' };

        int startX = x;
        for (int r = 0; r < buffer.GetHeight(); r++)
        {
            for (int c = 0; c < buffer.GetWidth(); c++)
            {
                drawStr[0] = buffer.GetAt({ r, c })->m_Char;
                if (drawStr[0] != EMPTY_CHAR_PLACEHOLDER)
                {
                    DrawText(drawStr, x, y, renderInfo.m_FontSize, buffer.GetAt({ r, c })->m_Color);
                }
               /*Utils::Log(std::format("Drawing character: {} at pos: {} with color: {}", 
                    Utils::ToString(drawStr[0]), Utils::Point2DInt(r, c).ToString(), RaylibUtils::ToString(buffer.GetAt({ r, c })->m_Color)));*/
                x += charArea.m_X;
            }

            y += charArea.m_Y;
            x = startX;
        }

        EndDrawing();
    }
}