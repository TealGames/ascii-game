#pragma once
#include <optional>
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"

namespace Rendering
{
    struct RenderInfo
    {
        std::optional<Utils::Point2DInt> m_CharSpacing;
        Utils::Point2DInt m_Padding;
        Utils::Point2DInt m_ScreenSize;
        std::uint16_t m_FontSize;
        bool m_CenterBuffer;
    };

    void RenderBuffer(const TextBuffer& buffer, const RenderInfo& renderInfo);
}